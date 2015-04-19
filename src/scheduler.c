/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▁ Documentation ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/**
 * @file   scheduler.c
 * @author Sébastien Rouault <sebmsg@free.fr>
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version. Please see https://gnu.org/licenses/gpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * @section DESCRIPTION
 *
 * Ordonnance les paquets et équilibre les nouvelles connexions entre routeurs (en gros).
**/

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Documentation ▔
/// ▁ Déclarations ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Headers externes
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <net/netfilter/nf_conntrack.h>

/// Headers internes
#include "hooks.h"
#include "scheduler.h"
#include "control.h"

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Déclarations ▔
/// ▁ Gestion des connexions ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Constantes
#define SCHEDULER_MAXPACKETQUEUESIZE FAIRCONF_SCHEDULER_MAXPACKETQUEUESIZE // Nombre de paquets par queue de connexion

/// Contrôle des constantes
#if FAIRCONF_SCHEDULER_MAXPACKETQUEUESIZE < 1
#error At least 1 packet per connection queue must be allowed
#endif

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Connexion
struct connection {
    struct access access; // Verrou d'accès
    nint version;   // Version d'IP
    nint lastsize;  // Taille du dernier paquet envoyé
    bool scheduled; // Connexion schedulée, donc ne devant pas être re-schedulée sur arrivée d'un autre paquet
    struct list_head  listmbr;     // Liste des connexions pour l'adhérent
    struct list_head  listgw;      // Liste des connexions sur le routeur
    struct list_head  listsched;   // Liste des connexions schedulées sur le routeur
    struct member*    member;      // Adhérent propriétaire
    struct router*    router;      // Routeur utilisé
    struct throughput throughup;   // Débit obtenu montant (en o/s)
    struct throughput throughdown; // Débit obtenu descendant (en o/s)
    struct {
        void (*timerfunc)(unsigned long); // Handler du timer dans netfilter (toujours death_by_timeout pour nous, mais le symbole n'est pas exporté)
        struct nf_conn* nfct; // Structure du conntrack associée (null si non liée)
    } conntrack; // Interface avec conntrack
    struct {
        nint count; // Nombre de paquets dans la table
        nint pos;   // Position dans la table tournante
        struct sk_buff* table[SCHEDULER_MAXPACKETQUEUESIZE]; // Table tournante des paquets
    } packets; // Gestion des paquets
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

ACCESS_DEFINE(connection, access); // Fonctions d'accès

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Libère la structure de connexion, destructeur de l'objet.
 * @param connection Structure de la connexion
 * @param dummy      Paramètre ignoré
**/
static void connection_destroy(struct connection* connection, zint dummy) {
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return;
    kmem_cache_free(scheduler.cacheconnections, connection); // Libération
    scheduler_unlock(&scheduler); /// UNLOCK
}

/** Initialise partiellement la structure de la connexion.
 * @param connection Structure de la connexion
 * @return Précise si l'opération est un succès
**/
static void connection_init(struct connection* connection) {
    connection->lastsize = 0;
    connection->scheduled = false;
    connection->packets.count = 0;
    connection->packets.pos = 0;
    connection->conntrack.nfct = null;
    connection->conntrack.timerfunc = null;
    throughput_init(&(connection->throughup));
    throughput_init(&(connection->throughdown));
    connection_open(connection, (void (*)(struct access*, zint)) connection_destroy, 0); // Ouverture de l'objet
}

/** Nettoie la structure de la connexion.
 * @param connection Structure de la connexion
 * @param needcall   Précise si la fonction du timer doit être appelée
**/
static void connection_clean(struct connection* connection, bool needcall) {
    struct member* member; // Ancien adhérent
    struct router* router; // Ancien routeur
    void (*timerfunc)(unsigned long) = null; // Handler du timer
    unsigned long timerdata; // Paramètre du handler du timer (la structure du conntrack)
    if (unlikely(!connection_lock(connection))) /// LOCK
        return;
    { // Suppression du lien avec conntrack
        struct nf_conn* ct = connection->conntrack.nfct; // Récupération de la connexion
        if (ct) { // Lié (null si non liée)
            if (needcall) { // Clean généré par netfilter
                timerfunc = connection->conntrack.timerfunc;
                timerdata = (unsigned long) ct;
            } else { // Clean généré par le scheduler
                setup_timer(&(ct->timeout), connection->conntrack.timerfunc, (unsigned long) ct); // Restauration du timer (au cas où), prise de référence déréférencé en fin de fonction
            }
        }
        connection->conntrack.nfct = null; // Marquée non liée
    }
    { // Flush des paquets
        nint i = connection->packets.pos; // Compteur
        nint limit = i + connection->packets.count; // Limite du compteur
        for (; i < limit; i++) // Pour tous les paquets
            consume_skb(connection->packets.table[i % SCHEDULER_MAXPACKETQUEUESIZE]); // Libération du paquet (drop d'une référence in fact)
    }
    member = connection->member; // Récupération de l'ancien adhérent
    connection->member = null; // Retrait de l'ancien adhérent, et prise de référence
    router = connection->router; // Récupération de l'ancien routeur
    connection->router = null; // Retrait de l'ancien routeur, et prise de référence
    connection_unlock(connection); /// UNLOCK
    connection_unref(connection); /// UNREF
    if (member) { // Détachement de l'ancien adhérent
        if (likely(member_lock(member))) { /// LOCK
            list_del(&(connection->listmbr)); // Sortie de la liste des connexions, prise de référence
            member->connections.count--; // Décompte de la connexion
            member_unlock(member); /// UNLOCK
        } // Sinon considérée comme détachée car member détruit
        member_unref(member); /// UNREF
        connection_unref(connection); /// UNREF
    }
    if (router) { // Détachement de l'ancien routeur
        if (likely(router_lock(router))) { /// LOCK
            list_del(&(connection->listgw)); // Sortie de la liste des connexions, prise de référence
            router_unlock(router); /// UNLOCK
        } // Sinon considérée comme détachée car router détruite
        router_unref(router); /// UNREF
        connection_unref(connection); /// UNREF
    }
    if (unlikely(!connection_lock(connection))) /// LOCK
        return;
    connection_close(connection); // Fermeture de l'objet
    connection_unlock(connection); /// UNLOCK
    if (timerfunc) // Appel nécessaire
        timerfunc(timerdata); // Appel du handler de netfilter
}

/** Alloue une nouvelle connexion et l'initialise partiellement.
 * @param flags Flags d'allocation
 * @return Pointeur sur la connexion allouée référencée, null si échec
**/
static struct connection* connection_create(gfp_t flags, nint version) {
    struct connection* connection; // Tuple
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return null;
    connection = kmem_cache_alloc(scheduler.cacheconnections, flags); // Allocation de la structure
    scheduler_unlock(&scheduler); /// UNLOCK
    if (unlikely(!connection)) // Échec d'allocation
        return null;
    connection_init(connection); // Initialisation
    connection->version = version; // Inscription de la version
    return connection;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Obtient l'adhérent référencé propriétaire de la connexion.
 * @param connection Structure de la connexion
 * @return Pointeur sur l'adhérent référencé, null si aucun/échec
**/
static struct member* connection_getmember(struct connection* connection) {
    struct member* member; // Adhérent en sortie
    if (unlikely(!connection_lock(connection))) /// LOCK
        return null;
    member = connection->member; // Récupération de l'adhérent
    if (!member) { // Aucun adhérent attaché
        connection_unlock(connection); /// UNLOCK
        return null;
    }
    member_ref(member); /// REF
    connection_unlock(connection); /// UNLOCK
    return member;
}

/** Obtient le routeur référencé propriétaire de la connexion.
 * @param connection Structure de la connexion
 * @return Pointeur sur le routeur référencé, null si aucun/échec
**/
static struct router* connection_getgateway(struct connection* connection) {
    struct router* router; // Adhérent en sortie
    if (unlikely(!connection_lock(connection))) /// LOCK
        return null;
    router = connection->router; // Récupération de l'adhérent
    if (!router) { // Aucun adhérent attaché
        connection_unlock(connection); /// UNLOCK
        return null;
    }
    router_ref(router); /// REF
    connection_unlock(connection); /// UNLOCK
    return router;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Ajoute un paquet en queue de la connexion.
 * @param connection Connexion concernée, verrouillée
 * @param skb        Socket buffer à mettre en queue
 * @return Précise si le paquet a bien été mis en queue
**/
static inline bool connection_push(struct connection* connection, struct sk_buff* skb) {
    if (connection->packets.count >= SCHEDULER_MAXPACKETQUEUESIZE) // Plus de place
        return false;
    connection->packets.table[(connection->packets.pos + (connection->packets.count++)) % SCHEDULER_MAXPACKETQUEUESIZE] = skb;
    return true;
}

/** Récupère le plus ancien paquet de la connexion.
 * @param connection Connexion concernée, verrouillée
 * @return Plus ancien socket buffer
**/
static inline struct sk_buff* connection_peek(struct connection* connection) {
    return connection->packets.table[connection->packets.pos]; // Récupération du plus ancien paquet
}

/** Récupère et retire le plus ancien paquet de la connexion.
 * @param connection Connexion concernée, verrouillée
 * @return Plus ancien socket buffer, retiré de la file
**/
static inline struct sk_buff* connection_pop(struct connection* connection) {
    struct sk_buff* skb;
    if (connection->packets.count == 0) // Aucun paquet
        return null;
    skb = connection_peek(connection); // Récupération du paquet
    connection->packets.pos = (connection->packets.pos + 1) % SCHEDULER_MAXPACKETQUEUESIZE; // Nouvelle position du plus ancien
    return skb;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Gestion des connexions ▔
/// ▁ Gestion des routeurs ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Ferme toutes les connexions du routeur, les connexions schedulées seront automatiquement rejetées.
 * @param router Structure du routeur, verrouillé
 * @return Précise si le routeur est toujours verrouillé
**/
static bool router_closeconnections(struct router* router) {
    struct list_head* list = &(router->connections.ipv4); // Liste des connexions IPv4
    struct connection* connection; // Connexion en cours
    while (!list_empty(list)) { // Au moins un élément à supprimer
        connection = container_of(list->next, struct connection, listmbr); // Récupération de l'adhérent
        connection_ref(connection); /// REF
        router_unlock(router); /// UNLOCK
        connection_clean(connection, false); // Nettoyage de la connexion
        connection_unref(connection); /// UNREF
        if (unlikely(!router_lock(router))) /// LOCK
            return false;
    }
    list = &(router->connections.ipv6); // Liste des connexions IPv6
    while (!list_empty(list)) { // Au moins un élément à supprimer
        connection = container_of(list->next, struct connection, listmbr); // Récupération de l'adhérent
        connection_ref(connection); /// REF
        router_unlock(router); /// UNLOCK
        connection_clean(connection, false); // Nettoyage de la connexion
        connection_unref(connection); /// UNREF
        if (unlikely(!router_lock(router))) /// LOCK
            return false;
    }
    return true;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Initialise la structure du routeur.
 * @param router Structure du routeur
 * @return Précise si l'opération est un succès
**/
bool router_init(struct router* router) {
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return false;
    list_add(&(router->list), &(scheduler.routers.offline)); // Offline par défaut
    scheduler_unlock(&scheduler); /// UNLOCK
    router->status = ROUTER_STATUS_OFFLINE; // Statut inconnu, offline par défaut
    router->allowipv4 = false; // Aucune adresse IPv4 attribuée
    router->allowipv6 = false; // Aucune adresse IPv6 attribuée
    router->throughlimit = 0; // Aucune limitation de débit
    throughput_init(&(router->throughup));   // Initialisation...
    throughput_init(&(router->throughdown)); // ...des débits...
    average_init(&(router->latency));        // ...et moyennes
    sortlist_init(&(router->connections.sortlist)); // Initialisation de la liste triée
    INIT_LIST_HEAD(&(router->members)); // Initialisation...
    INIT_LIST_HEAD(&(router->connections.ipv4));  // ...des...
    INIT_LIST_HEAD(&(router->connections.ipv6));  // ...listes
    router_open(router, (void (*)(struct access*, zint)) access_kfree, 0); // Ouverture de l'objet
    return true;
}

/** Nettoie la structure du routeur, ferme l'objet.
 * @param router Structure du routeur
**/
void router_clean(struct router* router) {
    if (unlikely(!router_lock(router))) /// LOCK
        return;
    if (unlikely(!router_closeconnections(router))) // Fermeture des connexions
        return; // Verrouillage perdu, routeur détruit
    { // Suppression en tant que routeur préféré
        struct list_head* list = &(router->members); // Liste à nettoyer
        struct member* member; // Adhérent en cours
        while (!list_empty(list)) { // Au moins un élément à supprimer
            member = container_of(list->next, struct member, router.list); // Récupération de l'adhérent
            member_ref(member); /// REF
            router_unlock(router); /// UNLOCK
            member_setgateway(member, null); // Aucun routeur préféré
            member_unref(member); /// UNREF
            if (unlikely(!router_lock(router))) /// LOCK
                return;
        }
    }
    router_close(router); // Fermeture de l'objet
    router_unlock(router); /// UNLOCK
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Change d'état (online/offline) le routeur, effectue la modification auprès du scheduler, peut clore des connexions.
 * @param router Structure du routeur
 * @param online  Vrai si le routeur doit être marqué online, offline sinon
 * @return Précise si l'opération est un succès
**/
bool router_setstatus(struct router* router, bool online) {
    bool router_setstatus_dochange(void) {
        if (unlikely(!scheduler_lock(&scheduler))) { /// LOCK
            if (likely(router_lock(router))) { /// LOCK
                router->status = online ? ROUTER_STATUS_OFFLINE : ROUTER_STATUS_ONLINE; // Restauration de l'état
                router_unlock(router); /// UNLOCK
            }
            return false;
        }
        list_move(&(router->list), online ? &(scheduler.routers.online) : &(scheduler.routers.offline)); // Changement de liste
        scheduler_unlock(&scheduler); /// UNLOCK
        return true;
    }
    if (unlikely(!router_lock(router))) /// LOCK
        return false;
    if (unlikely(router->status == ROUTER_STATUS_CLOSING || router->status == ROUTER_STATUS_CLOSED)) { // En cours de fermeture
        router_unlock(router); /// UNLOCK
        return false;
    }
    if (online) { // Mise online
        if (router->status == ROUTER_STATUS_OFFLINE) { // Depuis offline
            router->status = ROUTER_STATUS_ONLINE;
            router_unlock(router); /// UNLOCK
            return router_setstatus_dochange();
        }
    } else { // Mise offline
        if (router->status == ROUTER_STATUS_ONLINE) { // Depuis online
            router->status = ROUTER_STATUS_OFFLINE;
            if (unlikely(!router_closeconnections(router))) // Fermeture des connexions
                return false; // Verrouillage perdu, routeur détruit
            router_unlock(router); /// UNLOCK
            return router_setstatus_dochange();
        }
    }
    router_unlock(router); /// UNLOCK
    return true;
}

/** Entame, et peut terminer, la procédure de fermeture du routeur.
 * @param router Structure du routeur
 * @return Statut du routeur
**/
nint router_end(struct router* router) {
    if (unlikely(!router_lock(router))) /// LOCK
        return ROUTER_STATUS_CLOSED;
    switch (router->status) { // Selon le statut
        case ROUTER_STATUS_ONLINE: // Routeur fonctionnel
            if (sortlist_empty(&(router->connections.sortlist))) { // Aucun connexion
                router->status = ROUTER_STATUS_CLOSED;
                router_clean(router); // Nettoyage du routeur
                router_unlock(router); /// UNLOCK
                return ROUTER_STATUS_CLOSED;
            } else { // Au moins une connexion en cours
                router->status = ROUTER_STATUS_CLOSING;
                router_unlock(router); /// UNLOCK
                { // Changement de position
                    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
                        return ROUTER_STATUS_CLOSED; // Plus de scheduler...
                    list_move(&(router->list), &(scheduler.routers.closing)); // Déplacement du routeur
                    scheduler_unlock(&scheduler); /// UNLOCK
                }
                return ROUTER_STATUS_CLOSING;
            }
        case ROUTER_STATUS_OFFLINE: // Routeur hors-service
            router_clean(router); // Nettoyage du routeur
            router_unlock(router); /// UNLOCK
            return ROUTER_STATUS_CLOSED;
        case ROUTER_STATUS_CLOSING: // Déjà en cours de fermeture
            router_unlock(router); /// UNLOCK
            return ROUTER_STATUS_CLOSING;
        default: // Déjà fermée (anormal)
            router_unlock(router); /// UNLOCK
            return ROUTER_STATUS_CLOSED;
    }
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Modifie les prises en charges des versions d'IP du routeur, peut fermer des connexions.
 * @param router Structure du routeur
 * @param ipv4    Prise en charge IPv4
 * @param ipv6    Prise en charge IPv6
**/
void router_allowip(struct router* router, nint ipv4, nint ipv6) {
    if (ipv4 != ROUTER_ALLOWIP_UNCHANGE) { // Modification possible pour IPv4
        if (unlikely(!router_lock(router))) /// LOCK
            return;
        if (ipv4 != router->allowipv4) { // Modification effective
            router->allowipv4 = ipv4;
            if (ipv4 == ROUTER_ALLOWIP_NO) { // Terminaison des connexions IPv4
                struct connection* connection; // Connexion en cours
                while (!list_empty(&(router->connections.ipv4))) { // Pour toutes les connexions
                    connection = container_of(router->connections.ipv4.next, struct connection, listgw); // Connexion en cours
                    connection_ref(connection); /// REF
                    router_unlock(router); /// UNLOCK
                    connection_clean(connection, false); // Nettoyage de la connexion
                    connection_unref(connection); /// UNREF
                    if (unlikely(!router_lock(router))) /// LOCK
                        return;
                }
            }
        }
        router_unlock(router); /// UNLOCK
    }
    if (ipv6 != ROUTER_ALLOWIP_UNCHANGE) { // Modification possible pour IPv6
        if (unlikely(!router_lock(router))) /// LOCK
            return;
        if (ipv6 != router->allowipv6) { // Modification effective
            router->allowipv6 = ipv6;
            if (ipv6 == ROUTER_ALLOWIP_NO) { // Terminaison des connexions IPv4
                struct connection* connection; // Connexion en cours
                while (!list_empty(&(router->connections.ipv6))) { // Pour toutes les connexions
                    connection = container_of(router->connections.ipv6.next, struct connection, listgw); // Connexion en cours
                    connection_ref(connection); /// REF
                    router_unlock(router); /// UNLOCK
                    connection_clean(connection, false); // Nettoyage de la connexion
                    connection_unref(connection); /// UNREF
                    if (unlikely(!router_lock(router))) /// LOCK
                        return;
                }
            }
        }
        router_unlock(router); /// UNLOCK
    }
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Gestion des routeurs ▔
/// ▁ Gestion des adhérents ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Initialise la structure de l'adhérent.
 * @param member Structure de l'adhérent
**/
void member_init(struct member* member) {
    throughput_init(&(member->throughask));
    throughput_init(&(member->throughlost));
    throughput_init(&(member->throughup));
    throughput_init(&(member->throughdown));
    average_init(&(member->latency));
    INIT_LIST_HEAD(&(member->tuples));
    INIT_LIST_HEAD(&(member->router.list));
    INIT_LIST_HEAD(&(member->connections.list));
    member->router.structure = null; // Aucune structure favorite
    member->connections.count = 0; // Aucune connexion
    member->priority = MEMBER_BASEPRIORITY; // Priorité de base
    member_open(member, (void (*)(struct access*, zint)) access_kfree, 0); // Ouverture de l'objet
}

/** Nettoie la structure de l'adhérent ainsi que ses liens.
 * @param member Structure de l'adhérent
**/
void member_clean(struct member* member) {
    { // Suppression des tuples
        struct list_head* list = &(member->tuples); // Liste à nettoyer
        struct tuple* tuple; // Tuple en cours
        if (unlikely(!member_lock(member))) /// LOCK
            return;
        while (!list_empty(list)) { // Au moins un élément à supprimer
            tuple = container_of(list->next, struct tuple, list); // Récupération du tuple
            tuple_ref(tuple); /// REF
            member_unlock(member); /// UNLOCK
            tuple_clean(tuple); // Nettoyage du tuple
            tuple_unref(tuple); /// UNREF
            if (unlikely(!member_lock(member))) /// LOCK
                return;
        }
        member_unlock(member); /// UNLOCK
    }
    { // Terminaison des connexions
        struct list_head* list = &(member->connections.list);
        struct connection* connection; // Connexion en cours
        if (unlikely(!member_lock(member))) /// LOCK
            return;
        while (!list_empty(list)) { // Au moins un élément à supprimer
            connection = container_of(list->next, struct connection, listmbr); // Récupération de la connexion
            connection_ref(connection); /// REF
            member_unlock(member); /// UNLOCK
            connection_clean(connection, false); // Nettoyage de la connexion
            connection_unref(connection); /// UNREF
            if (unlikely(!member_lock(member))) /// LOCK
                return;
        }
        member_unlock(member); /// UNLOCK
    }
    member_setgateway(member, null); // Suppression du routeur préféré
    if (unlikely(!member_lock(member))) /// LOCK
        return;
    member_close(member); // Fermeture de l'objet
    member_unlock(member); /// UNLOCK
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Récupère le routeur préféré de l'adhérent.
 * @param member Structure de l'adhérent
 * @return Structure du routeur (null pour aucun)
**/
struct router* member_getgateway(struct member* member) {
    struct router* router; // Routeur en sortie
    if (unlikely(!member_lock(member))) /// LOCK
        return null;
    router = member->router.structure;
    member_unlock(member); /// UNLOCK
    return router;
}

/** Applique le routeur préféré de l'adhérent.
 * @param member  Structure de l'adhérent
 * @param router Structure du routeur (null pour aucun)
**/
void member_setgateway(struct member* member, struct router* router) {
    if (unlikely(!member_lock(member))) /// LOCK
        return;
    if (member->router.structure != router) { // Modifications à faire
        struct router* oldgateway = member->router.structure; // Ancien routeur
        if (oldgateway) { // Existante
            list_del(&(member->router.list)); // Sortie de la liste
            router_unref(oldgateway); /// UNREF
        }
        if (router) { // Nouveau routeur
            if (likely(router_isvalid(router))) { // Le routeur est valide
                router_ref(router); /// REF
                member->router.structure = router;
                list_add(&(member->router.list), &(router->members)); // Entrée dans la liste
            }
        } else {
            member->router.structure = null;
        }
    }
    member_unlock(member); /// UNLOCK
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Gestion des adhérents ▔
/// ▁ Gestion des tuples ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Libère le tuple, destructeur de l'objet.
 * @param tuple Structure du tuple
 * @param dummy Paramètre ignoré
**/
static void tuple_destroy(struct tuple* tuple, zint dummy) {
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return;
    kmem_cache_free(scheduler.cachetuples, tuple); // Libération
    scheduler_unlock(&scheduler); /// UNLOCK
}

/** Initialise la structure du tuple.
 * @param tuple Structure du tuple
**/
void tuple_init(struct tuple* tuple) {
    INIT_LIST_HEAD(&(tuple->list));
    tuple->member = null;
    INIT_LIST_HEAD(&(tuple->hash.macipv4));
    INIT_LIST_HEAD(&(tuple->hash.macipv6));
    tuple_open(tuple, (void (*)(struct access*, zint)) tuple_destroy, 0); // Ouverture de l'objet
}

/** Nettoie la structure du tuple.
 * @param tuple Structure du tuple
**/
void tuple_clean(struct tuple* tuple) {
    scheduler_removetuple(tuple); // Sortie du tuple
    if (unlikely(!tuple_lock(tuple))) /// LOCK
        return;
    { // Sortie de la chaîne du membre
        struct member* member = tuple->member;
        if (member) { // Existe
            tuple->member = null; // Suppression
            tuple_unlock(tuple); /// UNLOCK
            if (likely(member_lock(member))) { /// LOCK
                list_del(&(tuple->list)); // Prise de référence
                member_unlock(member); /// UNLOCK
            }
            tuple_unref(tuple); /// UNREF
            member_unref(member); /// UNREF
            if (unlikely(!tuple_lock(tuple))) /// LOCK
                return;
        }
    }
    tuple_close(tuple); // Fermeture de l'objet
    tuple_unlock(tuple); /// UNLOCK
}

/** Alloue un nouveau tuple, l'initialise, et le référence.
 * @param flags Flags d'allocation
 * @return Pointeur sur le tuple alloué référencé, null si échec
**/
struct tuple* tuple_create(gfp_t flags) {
    struct tuple* tuple; // Tuple
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return null;
    tuple = kmem_cache_alloc(scheduler.cachetuples, flags); // Allocation de la structure
    scheduler_unlock(&scheduler); /// UNLOCK
    if (unlikely(!tuple)) // Échec d'allocation
        return null;
    tuple_init(tuple); // Initialisation
    return tuple;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Gestion des tuples ▔
/// ▁ Gestion du scheduler ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Initialise la structure du scheduler.
 * @return Précise si l'opération est un succès
**/
bool scheduler_init(void) {
    scheduler.cacheconnections = kmem_cache_create("faircrave_connections", sizeof(struct connection), 0, 0, null); // Allocation du slab
    if (unlikely(!scheduler.cacheconnections)) // Échec d'allocation
        return false;
    scheduler.cachetuples = kmem_cache_create("faircrave_tuples", sizeof(struct tuple), 0, 0, null); // Allocation du slab
    if (unlikely(!scheduler.cacheconnections)) { // Échec d'allocation
        kmem_cache_destroy(scheduler.cachetuples); // Destruction du slab
        return false;
    }
    if (unlikely(!hooks_init())) { // Initialisation des hooks
        kmem_cache_destroy(scheduler.cacheconnections); // Destruction du slab
        kmem_cache_destroy(scheduler.cachetuples); // Destruction du slab
        return false;
    }
    scheduler.maxconnections = SCHEDULER_MAXCONNECTIONS;
    scheduler.throughput = 0;
    scheduler.inputfaces.count = 0;
    INIT_LIST_HEAD(&(scheduler.routers.online));
    INIT_LIST_HEAD(&(scheduler.routers.offline));
    INIT_LIST_HEAD(&(scheduler.routers.offline));
    spin_lock_init(&(scheduler.routers.onlock));
    { // Initialisation des maps
        struct scheduler_bucket* bucket; // Bucket en cours
        nint i; // Compteur
        { // Map MAC + IPv4 -> tuple
            bucket = scheduler.tuples.macipv4;
            for (i = 0; i < SCHEDULER_MAP_SIZE; i++) {
                spin_lock_init(&(bucket->lock));
                INIT_LIST_HEAD(&(bucket->list));
                bucket++; // Bucket suivant
            }
        }
        { // Map MAC + IPv6 -> tuple
            bucket = scheduler.tuples.macipv6;
            for (i = 0; i < SCHEDULER_MAP_SIZE; i++) {
                spin_lock_init(&(bucket->lock));
                INIT_LIST_HEAD(&(bucket->list));
                bucket++; // Bucket suivant
            }
        }
    }
    scheduler_open(&scheduler, null, 0); // Ouverture de l'objet statique
    return true;
}

/** Nettoie la structure du scheduler.
**/
void scheduler_clean(void) {
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return;
    hooks_clean(); // Nettoyage des hooks
    kmem_cache_destroy(scheduler.cacheconnections); // Destruction du slab
    kmem_cache_destroy(scheduler.cachetuples); // Destruction du slab
    /// TODO: Nettoyage du scheduler
    scheduler_close(&scheduler); // Fermeture de l'objet
    scheduler_unlock(&scheduler); /// UNLOCK
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Verrouille un bucket.
 * @param bucket Structure du bucket
 * @return Précise si l'opération est un succès
**/
static inline bool scheduler_bucket_lock(struct scheduler_bucket* bucket) {
    while (true) { // Acquisition du bucket
        if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
            return false;
        if (likely(spin_trylock(&(bucket->lock)) == 0)) { /// TRYLOCK
            scheduler_unlock(&scheduler); /// UNLOCK
            break;
        }
        scheduler_unlock(&scheduler); /// UNLOCK
    }
    return true;
}

/** Déverrouille un bucket.
 * @param bucket Structure du bucket
**/
static inline void scheduler_bucket_unlock(struct scheduler_bucket* bucket) {
    spin_unlock(&(bucket->lock)); /// UNLOCK
}

/** Calcule le hash du tuple (un simple XOR ici).
 * @param mac  Adresse MAC
 * @param ipv4 Adresse IPv4
 * @return Hash du tuple
**/
static inline nint16 scheduler_hashtuple_ipv4(nint16* mac, nint16* ipv4) {
    nint16 hash = 0; // Hash MAC + IPv4
    nint i; // Compteur
    for (i = 0; i < MAC_SIZE / sizeof(nint16); i++) // Hash de la MAC
        hash ^= mac[i];
    for (i = 0; i < IPV4_SIZE / sizeof(nint16); i++) // Hash de l'IP
        hash ^= ipv4[i];
    return hash;
}

/** Calcule le hash du tuple (un simple XOR ici).
 * @param mac  Adresse MAC
 * @param ipv6 Adresse IPv6
 * @return Hash du tuple
**/
static inline nint16 scheduler_hashtuple_ipv6(nint16* mac, nint16* ipv6) {
    nint16 hash = 0; // Hash MAC + IPv4
    nint i; // Compteur
    for (i = 0; i < MAC_SIZE / sizeof(nint16); i++) // Hash de la MAC
        hash ^= mac[i];
    for (i = 0; i < IPV6_SIZE / sizeof(nint16); i++) // Hash de l'IP
        hash ^= ipv6[i];
    return hash;
}

/** Récupère un tuple d'après ses adresses.
 * @param mac     Adresse MAC
 * @param ip      Adresse IPvX
 * @param version Version d'IP
 * @return Structure du tuple référencé, null si non trouvé
**/
struct tuple* scheduler_gettuple(nint8* mac, nint8* ip, nint version) {
    switch (version) {
        case 4: { // Version 4
            nint16 hash = scheduler_hashtuple_ipv4((nint16*) mac, (nint16*) ip); // Hashé MAC + IPv4
            struct scheduler_bucket* bucket = scheduler.tuples.macipv4 + hash; // Bucket concerné
            if (unlikely(!scheduler_bucket_lock(bucket))) /// LOCK
                return null;
            { // Récupération du tuple
                struct tuple* tuple; // Tuple en cours
                list_for_each_entry(tuple, &(bucket->list), hash.macipv4) { // Pour tous les tuples du bucket
                    if (unlikely(!tuple_lock(tuple))) /// LOCK
                        continue; // On saute ce tuple
                    if (memcmp(mac, tuple->address.mac, 6) != 0) // Comparaison des adresses MAC
                        goto NOMATCH4;
                    if (memcmp(ip, tuple->address.ipv4, 4) != 0) // Comparaison des adresses IPv4
                        goto NOMATCH4;
                    tuple_unlock(tuple); /// UNLOCK
#if FAIRCONF_SCHEDULER_MAP_REORDERBUCKET == 1
                    if (tuple->hash.macipv4.prev != &(bucket->list)) // N'est pas le premier élément
                        list_move(&(tuple->hash.macipv4), &(bucket->list)); // Mise en première position
#endif
                    tuple_ref(tuple); /// REF
                    scheduler_bucket_unlock(bucket); /// UNLOCK
                    return tuple;
                NOMATCH4:
                    tuple_unlock(tuple); /// UNLOCK
                }
            }
            scheduler_bucket_unlock(bucket); /// UNLOCK
        } return null; // Non trouvé
        case 6: { // Version 6
            nint16 hash = scheduler_hashtuple_ipv6((nint16*) mac, (nint16*) ip); // Hashé MAC + IPv6
            struct scheduler_bucket* bucket = scheduler.tuples.macipv6 + hash; // Bucket concerné
            if (unlikely(!scheduler_bucket_lock(bucket))) /// LOCK
                return null;
            { // Récupération du tuple
                struct tuple* tuple; // Tuple en cours
                list_for_each_entry(tuple, &(bucket->list), hash.macipv6) { // Pour tous les tuples du bucket
                    if (unlikely(!tuple_lock(tuple))) /// LOCK
                        continue; // On saute ce tuple
                    if (memcmp(mac, tuple->address.mac, 6) != 0) // Comparaison des adresses MAC
                        goto NOMATCH6;
                    if (memcmp(ip, tuple->address.ipv6, 16) != 0) // Comparaison des adresses IPv6
                        goto NOMATCH6;
                    tuple_unlock(tuple); /// UNLOCK
#if FAIRCONF_SCHEDULER_MAP_REORDERBUCKET == 1
                    if (tuple->hash.macipv6.prev != &(bucket->list)) // N'est pas le premier élément
                        list_move(&(tuple->hash.macipv6), &(bucket->list)); // Mise en première position
#endif
                    tuple_ref(tuple); /// REF
                    scheduler_bucket_unlock(bucket); /// UNLOCK
                    return tuple;
                NOMATCH6:
                    tuple_unlock(tuple); /// UNLOCK
                }
            }
            scheduler_bucket_unlock(bucket); /// UNLOCK
        } return null; // Non trouvé
        default: // Version inconnue
            log(KERN_ERR, "Unknow IP version %lu", version);
            return null;
    }
}

/** Ajoute un tuple dans la map correspondante.
 * @param tuple Structure du tuple
 * @return Précise si l'opération est un succès
**/
bool scheduler_inserttuple(struct tuple* tuple) {
    nint version; // Version IP du tuple
    { // Récupération de la version
        if (unlikely(!tuple_lock(tuple))) /// LOCK
            return false;
        version = tuple->address.version;
        tuple_unlock(tuple); /// UNLOCK
    }
    switch (version) {
        case 4: {
            nint16 hash; // Hash du tuple
            struct scheduler_bucket* bucket; // Bucket concerné
            { // Calcul du hashé
                if (unlikely(!tuple_lock(tuple))) /// LOCK
                    return false;
                hash = scheduler_hashtuple_ipv4((nint16*) tuple->address.mac, (nint16*) tuple->address.ipv4);
                tuple_unlock(tuple); /// UNLOCK
            }
            bucket = scheduler.tuples.macipv4 + hash; // Récupération du bucket
            if (unlikely(!scheduler_bucket_lock(bucket))) /// LOCK
                return false;
            tuple_ref(tuple); /// REF
            list_add(&(tuple->hash.macipv4), &(bucket->list)); // Ajout du tuple
            scheduler_bucket_unlock(bucket); /// UNLOCK
        } break;
        case 6: {
            nint16 hash; // Hash du tuple
            struct scheduler_bucket* bucket; // Bucket concerné
            { // Calcul du hashé
                if (unlikely(!tuple_lock(tuple))) /// LOCK
                    return false;
                hash = scheduler_hashtuple_ipv6((nint16*) tuple->address.mac, (nint16*) tuple->address.ipv6);
                tuple_unlock(tuple); /// UNLOCK
            }
            bucket = scheduler.tuples.macipv6 + hash; // Récupération du bucket
            if (unlikely(!scheduler_bucket_lock(bucket))) /// LOCK
                return false;
            tuple_ref(tuple); /// REF
            list_add(&(tuple->hash.macipv6), &(bucket->list)); // Ajout du tuple
            scheduler_bucket_unlock(bucket); /// UNLOCK
        } break;
        default:
            log(KERN_ERR, "Unknow IP version %lu", version);
            return false;
    }
    return true;
}

/** Retire le tuple de la map correspondante.
 * @param tuple Structure du tuple
**/
void scheduler_removetuple(struct tuple* tuple) {
    nint version; // Version d'IP
    struct scheduler_bucket* bucket; // Bucket concerné
    if (unlikely(!tuple_lock(tuple))) /// LOCK
        return;
    version = tuple->address.version;
    switch (version) { // Selon la version
        case 4: // IPv4
            if (unlikely(list_empty(&(tuple->hash.macipv4)))) { // Non inscrite
                tuple_unlock(tuple); /// UNLOCK
                return;
            }
            bucket = scheduler.tuples.macipv4 + scheduler_hashtuple_ipv4((nint16*) tuple->address.mac, (nint16*) tuple->address.ipv4);
            tuple_unlock(tuple); /// UNLOCK
            if (unlikely(!scheduler_bucket_lock(bucket))) /// LOCK
                return;
            list_del(&(tuple->hash.macipv4)); // Déréferencement en fin de fonction
            scheduler_bucket_unlock(bucket); /// UNLOCK
            break;
        case 6: // IPv6
            if (unlikely(list_empty(&(tuple->hash.macipv6)))) { // Non inscrite
                tuple_unlock(tuple); /// UNLOCK
                return;
            }
            bucket = scheduler.tuples.macipv6 + scheduler_hashtuple_ipv6((nint16*) tuple->address.mac, (nint16*) tuple->address.ipv6);
            tuple_unlock(tuple); /// UNLOCK
            if (unlikely(!scheduler_bucket_lock(bucket))) /// LOCK
                return;
            list_del(&(tuple->hash.macipv6)); // Déréferencement en fin de fonction
            scheduler_bucket_unlock(bucket); /// UNLOCK
            break;
        default:
            log(KERN_ERR, "Unknow IP version %lu", version);
            return;
    }
    tuple_unref(tuple); /// UNREF
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Gestion du scheduler ▔
/// ▁ Interface avec les hooks ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Sur terminaison d'une connexion.
 * @param connection Structure de la connexion, à déréférencer
**/
void scheduler_interface_onconnterminate(struct connection* connection) {
log(KERN_NOTICE, "[pass] connection = %016lx", (nint) connection);
    connection_clean(connection, true); // Nettoyage de la connexion avec appel du handler du timer
    connection_unref(connection); /// UNREF
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Crée une nouvelle connexion IP, lui attribue un routeur et marque la connexion.
 * @param skb     Socket buffer arrivant
 * @param ct      Structure de la connexion dans conntrack
 * @param version Version d'IP
 * @return Précise si l'opération est un succès
**/
bool scheduler_interface_input(struct sk_buff* skb, struct nf_conn* ct, nint version) {
    struct member* member; // Adhérent concerné
    struct router* router = null; // Routeur concerné
    struct connection* connection; // Connexion créée
    nint mark = mark; // Marque utilisée
log(KERN_NOTICE, "[....] skb = %016lx", (nint) skb);
    { // Récupération de l'adhérent, référencé
        nint8* mac = ((nint8*) skb_mac_header(skb)) + 6; // Adresse MAC
        nint8* ip = ((nint8*) skb_network_header(skb)) + (version == 4 ? 12 : 8); // Adresse IP
        struct tuple* tuple = scheduler_gettuple(mac, ip, version); // Récupération du tuple, référencé
        if (!tuple) // Tuple non trouvé
            return true; // Laisse passer pour local (mais ne passera pas le forward)
        if (unlikely(!tuple_lock(tuple))) { /// LOCK
            tuple_unref(tuple); /// UNREF
            return false;
        }
        member = tuple->member;
        member_ref(member); /// REF
        tuple_unlock(tuple); /// UNLOCK
        tuple_unref(tuple); /// UNREF
    }
    { // Contrôle du nombre de connexions
        nint maxconn;
        if (unlikely(!scheduler_lock(&scheduler))) { /// LOCK
            member_unref(member); /// UNREF
            return false;
        }
        maxconn = scheduler.maxconnections;
        scheduler_unlock(&scheduler); /// UNLOCK
        if (unlikely(!member_lock(member))) { /// LOCK
            member_unref(member); /// UNREF
            return false;
        }
        if (member->connections.count >= maxconn) { // Trop de connexions
            member_unlock(member); /// UNLOCK
            member_unref(member); /// UNREF
            return false;
        }
        member_unlock(member); /// UNLOCK
    }
    { // Sélection du routeur, référencé
        zint min = min, cur; // Débit minimal trouvé, débit en cours
        struct router* current; // Routeur en cours
        if (unlikely(!scheduler_lock(&scheduler))) { /// LOCK
            member_unref(member); /// UNREF
            return false;
        }
        spin_lock(&(scheduler.routers.onlock)); /// LOCK
        scheduler_unlock(&scheduler); /// UNLOCK
        list_for_each_entry(current, &(scheduler.routers.online), list) { // Pour chaque routeur online
            if (unlikely(!router_lock(current))) /// LOCK
                continue;
            cur = throughput_get(&(current->throughup)); // Calcul du débit montant
            if (!router || cur < min) { // Meilleur routeur
                min = cur;
                mark = control_getobjectgatewaybystructure(current)->id; // Récupération de la mark
                if (router)
                    router_unref(router); /// UNREF
                router = current;
                router_ref(router); /// REF
            }
            router_unlock(current); /// UNLOCK
        }
        spin_unlock(&(scheduler.routers.onlock)); /// UNLOCK
        if (unlikely(!router)) { // Aucun routeur online
            member_unref(member); /// UNREF
            return false;
        }
    }
    { // Création de la connexion et push du paquet
        connection = connection_create(GFP_ATOMIC, version); // Allocation et initialisation de la connexion, référencée
        if (!connection) { // Échec d'allocation
            router_unref(router); /// UNREF
            member_unref(member); /// UNREF
            return false;
        }
        if (unlikely(!connection_lock(connection))) { /// LOCK
            connection_unref(connection); /// UNREF
            router_unref(router); /// UNREF
            member_unref(member); /// UNREF
            return false;
        }
        connection_push(connection, skb); // Push du paquet (ne peut pas échouer car aucun paquet dans la file)
        connection->router = router;
        connection->member = member;
        connection->conntrack.nfct = ct; // Référencement de la struction
        connection->conntrack.timerfunc = ct->timeout.function; // Sauvegarde de la fonction
        router_ref(router); /// REF
        member_ref(member); /// REF
        connection_unlock(connection); /// UNLOCK
    }
    { // Inscription auprès de l'adhérent
        if (unlikely(!member_lock(member))) { /// LOCK
            connection_unref(connection); /// UNREF
            router_unref(router); /// UNREF
            router_unref(router); /// UNREF
            member_unref(member); /// UNREF
            member_unref(member); /// UNREF
            return false;
        }
        member->connections.count++; // L'adhérent ouvre une nouvelle connexion
        connection_ref(connection); /// REF
        list_add(&(connection->listmbr), &(member->connections.list)); // Ajout à la liste
        member_unlock(member); /// UNLOCK
    }
    { // Inscription auprès du routeur
        if (unlikely(!router_lock(router))) { /// LOCK
            if (likely(member_lock(member))) { /// LOCK
                list_del(&(connection->listmbr)); // Sortie de la liste
                member_unlock(member); /// UNLOCK
                connection_unref(connection); /// UNREF
            }
            connection_unref(connection); /// UNREF
            router_unref(router); /// UNREF
            router_unref(router); /// UNREF
            member_unref(member); /// UNREF
            member_unref(member); /// UNREF
            return false;
        }
        connection_ref(connection); /// REF
        list_add(&(connection->listgw), version == 4 ? &(router->connections.ipv4) : &(router->connections.ipv6)); // Ajout de la connexion dans la liste
        router_unlock(router); /// UNLOCK
    }
    router_unref(router); /// UNREF
    member_unref(member); /// UNREF
    { // Traitement de la connexion
        ct->mark = mark; // Affectation de la mark
        setup_timer(&(ct->timeout), (void (*)(unsigned long)) scheduler_interface_onconnterminate, (unsigned long) connection); // Prise de référence
    }
log(KERN_NOTICE, "[pass] skb = %016lx", (nint) skb);
    return true;
}

/** Contrôle que le paquet forwardé correspond bien à un adhérent et un routeur.
 * @param skb     Socket buffer arrivant
 * @param ct      Structure de la connexion dans conntrack
 * @param version Version d'IP
 * @return Vrai si le paquet peut passer, faux sinon.
**/
bool scheduler_interface_forward(struct sk_buff* skb, struct nf_conn* ct, nint version) {
    if ((nint) ((struct nf_conn*) (skb->nfct))->timeout.data == (nint) ct) { // N'a pas été modifié (voir __nf_conntrack_alloc)
log(KERN_NOTICE, "[fail] skb = %016lx", (nint) skb);
        return false;
    }
log(KERN_NOTICE, "[pass] skb = %016lx", (nint) skb);
    return true;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――



/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Interface avec les hooks ▔
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
