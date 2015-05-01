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
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <net/netfilter/nf_conntrack.h>
#include <uapi/linux/ip.h>
#include <uapi/linux/in.h>

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
    nint   version;   // Version d'IP
    nint   lastsize;  // Taille du dernier paquet envoyé
    nint16 protocol;  // N° du protocole de niveau 4, endianness de la machine
    bool   scheduled; // Connexion schedulée, donc ne devant pas être re-schedulée sur arrivée d'un autre paquet
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
            if (needcall) { // Clean généré par le timer
                timerfunc = connection->conntrack.timerfunc;
                timerdata = (unsigned long) ct;
            } else { // Clean généré par le scheduler
                setup_timer(&(ct->timeout), connection->conntrack.timerfunc, (unsigned long) ct); // Restauration du timer (au cas où), prise de référence déréférencé en fin de fonction
            }
            connection->conntrack.nfct = null; // Marquée non liée
            nf_conntrack_put((struct nf_conntrack*) ct); /// UNREF
        }
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
static struct connection* connection_create(gfp_t flags) {
    struct connection* connection; // Tuple
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return null;
    connection = kmem_cache_alloc(scheduler.cacheconnections, flags); // Allocation de la structure
    scheduler_unlock(&scheduler); /// UNLOCK
    if (unlikely(!connection)) // Échec d'allocation
        return null;
    connection_init(connection); // Initialisation
    return connection;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Ajoute un paquet en queue de la connexion, ne schedule pas la connexion.
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
    connection->packets.count--;
    return skb;
}

/** Précise si le paquet est un paquet IP.
 * @param skb Socket buffer du paquet
 * @return Vrai s'il s'agit d'un paquet IP, faux sinon
**/
static inline bool connection_isip(struct sk_buff* skb) {
    struct ethhdr* ethhdr = (struct ethhdr*) skb_transport_header(skb); // Niveau Ethernet
    if (unlikely(ethhdr->h_proto != htons(ETH_P_IP))) // N'est pas un paquet IP
        return false;
    return true;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** (Re)-schedule la connexion, déverrouille l'objet.
 * @param connection Connexion concernée, verrouillée
 * @return Précise si l'opération est un succès
**/
static bool connection_schedule(struct connection* connection) {
    nint lastsize; // Taille du dernier paquet envoyé
    nint conncount; // Nombre de connexions de l'adhérent
    nint membertput; // Débit sortant de l'adhérents
    nint priority; // Priorité de l'adhérent
    struct member* member; // Adhérent propriétaire de la connexion
    struct router* router; // Routeur propriétaire de la connexion
    if (unlikely(connection->packets.count == 0)) { // Pas de rescheduling
        connection->scheduled = false;
        return true;
    }
    { // Récupération des paramètres
        lastsize = connection->lastsize;
        member = connection->member;
        router = connection->router;
        member_ref(member); /// REF
        router_ref(router); /// REF
        connection_unlock(connection); /// UNLOCK
        if (unlikely(!member_lock(member))) { /// LOCK
            connection->scheduled = false; // Sans verrouillage
            member_unref(member); /// UNREF
            router_unref(router); /// UNREF
            return false;
        }
        membertput = throughput_get(&(member->throughup));
        priority = member->priority;
        conncount = member->connections.count;
        member_unlock(member); /// UNLOCK
        member_unref(member); /// UNREF
    }
    { // Scheduling de la connexion
        nint retard; // Retard à appliquer
#if FAIRCONF_SCHEDULER_DEBUGSATURATE == 1
        bool saturate; // La connexion a été schedulée trop tôt, cause de saturation de la valeur de retard
#endif
        if (unlikely(!router_lock(router))) { /// LOCK
            connection->scheduled = false; // Au cas où ce n'était pas déjà le cas
            router_unref(router); /// UNREF
            return false;
        }
        retard = (lastsize * membertput * conncount * priority) / (router->netdev.mtu * scheduler.throughput * MEMBER_BASEPRIORITY);
#if FAIRCONF_SCHEDULER_DEBUGSATURATE == 1
        saturate = !sortlist_push(&(router->connections.sortlist), &(connection->listsched), retard); // Saturation ?
#else
        sortlist_push(&(router->connections.sortlist), &(connection->listsched), retard);
#endif
        router_unlock(router); /// UNLOCK
        router_unref(router); /// UNREF
        connection->scheduled = true; // Au cas où ce n'était pas déjà le cas
#if FAIRCONF_SCHEDULER_DEBUGSATURATE == 1
        if (unlikely(saturate)) { // Décompte de la saturation
            static nint count = 0; // Compte de saturation
            if ((count++) % FAIRCONF_SCHEDULER_DEBUGSATURATE_DELTA == 0)
                log(KERN_WARNING, "%lu connections have saturated scheduling sortlist", count);
        }
#endif
    }
    return true;
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
    list_add(&(router->list), &(scheduler.routers.standby)); // Standby par défaut
    scheduler_unlock(&scheduler); /// UNLOCK
    router->online = false; // Offline
    router->reachable = false; // Non accessible
    router->closing = false; // Pas en cours de fermeture
    router->allowipv4 = false; // Aucune adresse IPv4 attribuée
    router->allowipv6 = false; // Aucune adresse IPv6 attribuée
    router->throughlimit = 0; // Aucune limitation de débit
    router->netdev.ptr = null;
    router->netdev.mtu = 0;
    throughput_init(&(router->throughup));   // Initialisation...
    throughput_init(&(router->throughdown)); // ...des débits...
    average_init(&(router->latency));        // ...et moyennes
    sortlist_init(&(router->connections.sortlist)); // Initialisation de la liste triée
    INIT_LIST_HEAD(&(router->members)); // Initialisation...
    INIT_LIST_HEAD(&(router->connections.ipv4));  // ...des...
    INIT_LIST_HEAD(&(router->connections.ipv6));  // ...listes
    INIT_LIST_HEAD(&(router->netdev.list));
    router_open(router, (void (*)(struct access*, zint)) access_kfree, 0); // Ouverture de l'objet
    return true;
}

/** Nettoie la structure du routeur, ferme l'objet.
 * @param router Structure du routeur
**/
void router_clean(struct router* router) {
    router_setnetdev(router, null); // Suppression de la netdev liée
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
            member_setrouter(member, null); // Aucun routeur préféré
            member_unref(member); /// UNREF
            if (unlikely(!router_lock(router))) /// LOCK
                return;
        }
    }
    { // Déréférencement des connexions dans la sortlist
        struct connection* connection; // Connexion en cours
        for (;;) {
            connection = (struct connection*) sortlist_pop(&(router->connections.sortlist));
            if (!connection) // Sortlist vide
                break;
            connection = container_of((struct list_head*) connection, struct connection, listsched); // Récupération de la connexion
            connection_unref(connection); /// UNREF
        }
    }
    router_close(router); // Fermeture de l'objet
    router_unlock(router); /// UNLOCK
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Change l'état du routeur auprès du scheduler, peut clore des connexions, déverrouille la structure du routeur.
 * @param router Structure du routeur, verrouillé
 * @param ready  Si vrai le routeur est passe de non fonctionnel à fonctionnel, l'inverse sinon
 * @return Précise si l'opération est un succès
**/
static bool router_setstatus(struct router* router, bool ready) {
    struct netdev* netdev = router->netdev.ptr; // Network device liée
    struct list_head* list; // Liste cible
    zint delta; // Différence de débit à appliquer
    if (ready) { // Ajout du routeur
        delta = router->throughlimit; // Compté positivement
        list = &(scheduler.routers.ready);
    } else { // Suppression du routeur
        delta = -router->throughlimit; // Compté négativement
        list = &(scheduler.routers.standby);
        if (unlikely(!router_closeconnections(router))) // Fermeture des connexion, UNLOCK si faux
            return false;
    }
    if (netdev) { // Changement de chaîne dans la netdev
        if (unlikely(!netdev_lock(netdev))) /// LOCK
            return false;
        if (router_isready(router)) { // Routeur passe en "ready"
            list_move_tail(&(router->netdev.list), &(netdev->rdlist));
            netdev->count++;
        } else { // Routeur passe "en stand-by"
            list_move_tail(&(router->netdev.list), &(netdev->sblist));
            netdev->count--;
        }
        netdev_unlock(netdev); /// UNLOCK
    }
    router_unlock(router); /// UNLOCK
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return false;
    list_move(&(router->list), list); // Changement de liste
    scheduler.throughput += delta; // Application de la différence de débit
    scheduler_unlock(&scheduler); /// UNLOCK
    return true;
}

/** Change l'état du routeur, peut clore des connexions.
 * @param router Structure du routeur
 * @param online Le routeur est fonctionnel
 * @return Précise si l'opération est un succès
**/
bool router_setonline(struct router* router, bool online) {
    if (unlikely(!router_lock(router))) /// LOCK
        return false;
    if (router->closing) { // En cours de fermeture
        router_unlock(router); /// UNLOCK
        return false;
    }
    if (!router->online && online) { // Passage en online
        router->online = true;
        if (router->reachable && !router->closing) // Changement de disposition
            return router_setstatus(router, true);
    } else if (router->online && !online) { // Passage en offline
        router->online = false;
        if (router->reachable && !router->closing) // Changement de disposition
            return router_setstatus(router, false);
    }
    router_unlock(router); /// UNLOCK
    return true;
}

/** Change la network device associée au routeur, peut clore des connexions.
 * @param router Structure du routeur
 * @param netdev Network device à associer (null pour supprimer)
 * @return Précise si l'opération est un succès
**/
bool router_setnetdev(struct router* router, struct netdev* newnetdev) {
    if (unlikely(!router_lock(router))) /// LOCK
        return false;
    if (router->netdev.ptr != newnetdev) { // Modification nécessaire
        struct netdev* oldnetdev; // Ancienne netdev
        oldnetdev = router->netdev.ptr; // Récupération, prise de référence
        if (oldnetdev) { // Sortie nécessaire
            if (unlikely(!netdev_lock(oldnetdev))) /// LOCK
                return false;
            list_del(&(router->netdev.list));
            if (router_isready(router)) // Routeur était "ready"
                oldnetdev->count--;
            if (list_empty(&(oldnetdev->rdlist)) && list_empty(&(oldnetdev->sblist))) { // Suppression nécessaire
                scheduler_delnetdev(oldnetdev); /// UNLOCK
            } else {
                netdev_unlock(oldnetdev); /// UNLOCK
            }
            netdev_unref(oldnetdev); /// UNREF
            router_unref(router); /// UNREF
        }
        if (newnetdev) { // Entrée nécessaire
            router->reachable = true;
            if (unlikely(!netdev_lock(newnetdev))) /// LOCK
                return false;
            if (router_isready(router)) { // Routeur sera "ready"
                list_add_tail(&(router->netdev.list), &(newnetdev->rdlist)); // Référencement à la fin
                newnetdev->count++;
            } else { // Routeur "en stand-by"
                list_add_tail(&(router->netdev.list), &(newnetdev->sblist)); // Référencement à la fin
            }
            router->netdev.mtu = (nint) newnetdev->netdev->mtu; // Récupération de la MTU
            netdev_unlock(newnetdev); /// UNLOCK
            router->netdev.ptr = newnetdev;
            netdev_ref(newnetdev); /// REF
            router_ref(router); /// REF
        } else { // Suppression
            router->reachable = false;
        }
        router_unlock(router); /// UNLOCK
        return true;
    }
    router_unlock(router); /// UNLOCK
    return true;
}

/** Entame, et peut terminer, la procédure de fermeture du routeur.
 * @param router Structure du routeur
**/
void router_end(struct router* router) {
    if (unlikely(!router_lock(router))) /// LOCK
        return;
    if (!sortlist_empty(&(router->connections.sortlist))) { // Au moins une connexion en cours
        router->closing = true;
        if (router->online && router->reachable) // Changement d'état
            router_setstatus(router, false); /// UNLOCK
        return;
    }
    router_clean(router); // Nettoyage du routeur
    router_unlock(router); /// UNLOCK
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Modifie les prises en charges des versions d'IP du routeur, peut fermer des connexions.
 * @param router Structure du routeur
 * @param ipv4    Prise en charge IPv4
 * @param ipv6    Prise en charge IPv6
**/
void router_allowip(struct router* router, nint ipv4, nint ipv6) {
    if (ipv4 != ROUTER_UNCHANGE) { // Modification possible pour IPv4
        bool allow = (ipv4 == ROUTER_YES); // Autorisé ?
        if (unlikely(!router_lock(router))) /// LOCK
            return;
        if (router->allowipv4 != allow) { // Modification effective
            router->allowipv4 = allow;
            if (!allow) { // Terminaison des connexions IPv4
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
    if (ipv6 != ROUTER_UNCHANGE) { // Modification possible pour IPv6
        bool allow = (ipv6 == ROUTER_YES); // Autorisé ?
        if (unlikely(!router_lock(router))) /// LOCK
            return;
        if (router->allowipv6 != allow) { // Modification effective
            router->allowipv6 = allow;
            if (!allow) { // Terminaison des connexions IPv6
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
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
    throughput_init(&(member->throughask));
    throughput_init(&(member->throughlost));
#endif
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
            tuple = list_first_entry(list, struct tuple, list); // Récupération du tuple
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
            connection = list_first_entry(list, struct connection, listmbr); // Récupération de la connexion
            connection_ref(connection); /// REF
            member_unlock(member); /// UNLOCK
            connection_clean(connection, false); // Nettoyage de la connexion
            connection_unref(connection); /// UNREF
            if (unlikely(!member_lock(member))) /// LOCK
                return;
        }
        member_unlock(member); /// UNLOCK
    }
    member_setrouter(member, null); // Suppression du routeur préféré
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
struct router* member_getrouter(struct member* member) {
    struct router* router; // Routeur en sortie
    if (unlikely(!member_lock(member))) /// LOCK
        return null;
    router = member->router.structure;
    member_unlock(member); /// UNLOCK
    return router;
}

/** Applique le routeur préféré de l'adhérent.
 * @param member Structure de l'adhérent
 * @param router Structure du routeur (null pour aucun)
**/
void member_setrouter(struct member* member, struct router* router) {
    if (unlikely(!member_lock(member))) /// LOCK
        return;
    if (member->router.structure != router) { // Modifications à faire
        struct router* oldrouter = member->router.structure; // Ancien routeur
        if (oldrouter) { // Existante
            list_del(&(member->router.list)); // Sortie de la liste
            router_unref(oldrouter); /// UNREF
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
    spin_lock_init(&(scheduler.routers.lock));
    INIT_LIST_HEAD(&(scheduler.routers.ready));
    INIT_LIST_HEAD(&(scheduler.routers.standby));
    INIT_LIST_HEAD(&(scheduler.netdevs.list));
    spin_lock_init(&(scheduler.netdevs.lock));
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

/** Nettoie la structure du scheduler, après que les adhérents et routeurs aient été supprimés.
**/
void scheduler_clean(void) {
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return;
    hooks_clean(); // Nettoyage des hooks
    kmem_cache_destroy(scheduler.cacheconnections); // Destruction du slab
    kmem_cache_destroy(scheduler.cachetuples); // Destruction du slab
    scheduler_close(&scheduler); // Fermeture de l'objet
    scheduler_unlock(&scheduler); /// UNLOCK
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Récupère la netdev associée à une network device réelle, la crée si nécessaire.
 * @param net_device Network device réelle
 * @return Network device interne (null si échec), référencée
**/
struct netdev* scheduler_getnetdev(struct net_device* net_device) {
    struct netdev* netdev; // Network device trouvée
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return null;
    { // Recherche du netdev
        spin_lock(&(scheduler.netdevs.lock)); /// LOCK
        scheduler_unlock(&scheduler); /// UNLOCK
        list_for_each_entry(netdev, &(scheduler.netdevs.list), ndlist) { // Passage sans verrouillage (netdev n'est écrit qu'une fois, avant mise en chaîne)
            if (netdev->netdev == net_device) { // Trouvé
                netdev_ref(netdev); /// REF
                spin_unlock(&(scheduler.netdevs.lock)); /// UNLOCK
                return netdev;
            }
        }
        if (unlikely(!scheduler_lock(&scheduler))) { /// LOCK
            spin_unlock(&(scheduler.netdevs.lock)); /// UNLOCK
            return null;
        }
        spin_unlock(&(scheduler.netdevs.lock)); /// UNLOCK
    }
    { // Création du netdev
        netdev = kmalloc(sizeof(struct netdev), GFP_ATOMIC);
        if (unlikely(!netdev)) {
            scheduler_unlock(&scheduler); /// UNLOCK
            return null;
        }
        netdev->netdev = net_device;
        dev_hold(net_device); // Compte d'une référence
        INIT_LIST_HEAD(&(netdev->rdlist));
        INIT_LIST_HEAD(&(netdev->sblist));
        netdev->count = 0;
        netdev_open(netdev, access_kfree, 0); // Ouverture de l'objet
        spin_lock(&(scheduler.netdevs.lock)); /// LOCK
        list_add(&(netdev->ndlist), &(scheduler.netdevs.list));
        netdev_ref(netdev); /// REF
        spin_unlock(&(scheduler.netdevs.lock)); /// UNLOCK
    }
    scheduler_unlock(&scheduler); /// UNLOCK
    return netdev;
}

/** Supprime une netdev qui n'a plus de routeur lié, déverrouille la network device.
 * @param netdev Network device, verrouillée
**/
void scheduler_delnetdev(struct netdev* netdev) {
    dev_put(netdev->netdev); // Décompte d'une référence
    netdev_close(netdev); // Fermeture de l'objet
    netdev_unlock(netdev); /// UNLOCK
    if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
        return;
    spin_lock(&(scheduler.netdevs.lock)); /// LOCK
    scheduler_unlock(&scheduler); /// UNLOCK
    list_del(&(netdev->ndlist));
    netdev_unref(netdev); /// UNREF
    spin_unlock(&(scheduler.netdevs.lock)); /// UNLOCK
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
                    if (memcmp(mac, tuple->address.mac, MAC_SIZE) != 0) // Comparaison des adresses MAC
                        goto NOMATCH4;
                    if (memcmp(ip, tuple->address.ipv4, IPV4_SIZE) != 0) // Comparaison des adresses IPv4
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
                    if (memcmp(mac, tuple->address.mac, MAC_SIZE) != 0) // Comparaison des adresses MAC
                        goto NOMATCH6;
                    if (memcmp(ip, tuple->address.ipv6, IPV6_SIZE) != 0) // Comparaison des adresses IPv6
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
    if (unlikely(!tuple_lock(tuple))) /// LOCK
        return false;
    version = tuple->address.version; // Récupération de la version
    tuple_unlock(tuple); /// UNLOCK
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
            list_add(&(tuple->hash.macipv4), &(bucket->list)); // Ajout du tuple, référencement en fin de fonction
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
            list_add(&(tuple->hash.macipv6), &(bucket->list)); // Ajout du tuple, référencement en fin de fonction
            scheduler_bucket_unlock(bucket); /// UNLOCK
        } break;
        default:
            log(KERN_ERR, "Unknow IP version %lu", version);
            return false;
    }
    tuple_ref(tuple); /// REF
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
log(KERN_DEBUG, "Netfilter notify connection termination (%p)", connection);
    connection_clean(connection, true); // Nettoyage de la connexion avec appel du handler du timer
    connection_unref(connection); /// UNREF
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// TODO: Tests, tests, et encore des tests

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
        spin_lock(&(scheduler.routers.lock)); /// LOCK
        scheduler_unlock(&scheduler); /// UNLOCK
        list_for_each_entry(current, &(scheduler.routers.ready), list) { // Pour chaque routeur online accessible
            if (unlikely(!router_lock(current))) /// LOCK
                continue;
            cur = throughput_get(&(current->throughup)); // Calcul du débit montant
            if (!router || cur < min) { // Meilleur routeur
                min = cur;
                mark = control_getobjectrouterbystructure(current)->id; // Récupération de la mark
                if (router)
                    router_unref(router); /// UNREF
                router = current;
                router_ref(router); /// REF
            }
            router_unlock(current); /// UNLOCK
        }
        spin_unlock(&(scheduler.routers.lock)); /// UNLOCK
        if (unlikely(!router)) { // Aucun routeur online accessible
            member_unref(member); /// UNREF
            return false;
        }
    }
    { // Création de la connexion et push du paquet
        connection = connection_create(GFP_ATOMIC); // Allocation et initialisation de la connexion, référencée
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
        connection->version = version;
        connection->protocol = (nint) ntohs(((struct iphdr*) skb_network_header(skb))->protocol); // N° du protocol, endianness de la machine
        connection->router = router;
        connection->member = member;
        nf_conntrack_get((struct nf_conntrack*) ct); /// REF
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
    { // Traitement du paquet et de la connexion
        ct->mark = mark; // Affectation de la mark
        setup_timer(&(ct->timeout), (void (*)(unsigned long)) scheduler_interface_onconnterminate, (unsigned long) connection); // Prise de référence
    }
    return true;
}

/** Contrôle que le paquet forwardé correspond bien à un adhérent et un routeur.
 * @param skb     Socket buffer arrivant
 * @param ct      Structure de la connexion dans conntrack
 * @param version Version d'IP
 * @return Vrai si le paquet peut passer, faux sinon.
**/
bool scheduler_interface_forward(struct sk_buff* skb, struct nf_conn* ct, nint version) {
    if ((nint) ((struct nf_conn*) (skb->nfct))->timeout.data == (nint) ct) // N'a pas été modifié (voir __nf_conntrack_alloc)
        return false;
    return true;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Met en file le paquet, en admettant que le paquet a une structure nf_conn valide associée.
 * @param skb  Socket buffer arrivant
 * @param nfct Structure de la connexion dans netfilter
 * @return Vrai si le paquet a été mis en file, faux sinon.
**/
bool as(hot) scheduler_interface_enqueue(struct sk_buff* skb, struct nf_conn* nfct) {
    struct connection* connection = (struct connection*) nfct->timeout.data; // Connexion associée
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
    zint size = (zint) skb->truesize; // Taille du paquet
    struct member* member; // Adhérent propriétaire de la connexion
#endif
    if ((nint) connection == (nint) skb->nfct) // La connexion n'est gérée que par netfilter (voir __nf_conntrack_alloc)
        return false;
    if (unlikely(!connection_lock(connection))) /// LOCK
        return false;
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
    member = connection->member; // Récupération de l'adhérent
    member_ref(member); /// REF
#endif
    if (unlikely(!connection_push(connection, skb))) { // Échec du push du paquet en file (saturation de la file)
        connection_unlock(connection); /// UNLOCK
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
        if (likely(member_lock(member))) { /// LOCK
            throughput_add(&(member->throughask), size);
            throughput_add(&(member->throughlost), size);
            member_unlock(member); /// UNLOCK
            member_unref(member); /// UNREF
        }
        member_unref(member); /// UNREF
#endif
        return false;
    }
    if (!connection->scheduled) { // Pas encore schedulée
        connection_schedule(connection); /// UNLOCK
    } else {
        connection_unlock(connection); /// UNLOCK
    }
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
    if (unlikely(!member_lock(member))) { /// LOCK
        member_unref(member); /// UNREF
        return false; // Plus d'adhérent -> plus de connexion
    }
    throughput_add(&(member->throughask), size);
    member_unlock(member); /// UNLOCK
    member_unref(member); /// UNREF
#endif
    return true;
}

/** Récupère le premier paquet à envoyer pour ce routeur, sans aucun contrôle.
 * @param router Routeur dont le premier socket buffer à envoyer doit être peek.
 * @return Socket buffer (null si aucun/échec)
**/
struct sk_buff* as(hot) scheduler_interface_peek(struct router* router) {
    struct sk_buff* skb; // Socket buffer en sortie
    struct connection* connection; // Connexion concernée
    if (unlikely(!router_lock(router))) /// LOCK
        return null;
    connection = (struct connection*) sortlist_get(&(router->connections.sortlist)); // Connexion prioritaire (sur le list_head)
    if (unlikely(!connection)) { // Aucune connexion schedulée
        router_unlock(router); /// UNLOCK
        return null;
    }
    connection = container_of((struct list_head*) connection, struct connection, listsched); // Connexion prioritaire (sur la structure)
    connection_ref(connection); /// REF
    router_unlock(router); /// UNLOCK
    if (unlikely(!connection_lock(connection))) { /// LOCK
        connection_unref(connection); /// UNREF
        return null;
    }
    skb = connection_peek(connection); // Peek de premier paquet (s'il existe)
    connection_unlock(connection); /// UNLOCK
    connection_unref(connection); /// UNREF
    return skb;
}

/** Sort de la file le premier paquet pour ce routeur, limite le débit, met à jour les statistiques.
 * @param router Routeur dont le premier socket buffer à envoyer doit être pop.
 * @return Socket buffer (null si aucun/échec)
**/
struct sk_buff* as(hot) scheduler_interface_dequeue(struct router* router) {
    struct sk_buff* skb; // Socket buffer en sortie
    struct connection* connection; // Connexion concernée
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
    struct member* member; // Adhérent propriétaire de la connexion
    nint maxlatency; // Latence maximale admissible
#endif
    if (unlikely(!router_lock(router))) /// LOCK
        return null;
    { // Limitation du débit
        zint throughput = throughput_get(&(router->throughup));
        if (throughput >= router->throughlimit) { // Dépassement
            router_unlock(router); /// UNLOCK
            return null;
        }
    }
    connection = (struct connection*) sortlist_pop(&(router->connections.sortlist)); // Connexion prioritaire (sur le list_head)
    if (unlikely(!connection)) { // Aucune connexion schedulée
        router_unlock(router); /// UNLOCK
        return null;
    }
    connection = (struct connection*) container_of((struct list_head*) connection, struct connection, listsched); // Connexion prioritaire (sur la structure)
    connection_ref(connection); /// REF
    router_unlock(router); /// UNLOCK
    if (unlikely(!connection_lock(connection))) { /// LOCK
        connection_unref(connection); /// UNREF
        return null;
    }
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
    member = connection->member;
    member_ref(member); /// REF
    connection_unlock(connection); /// UNLOCK
    if (unlikely(!member_lock(member))) { /// LOCK
        connection_unref(connection); /// UNREF
        member_unref(member); /// UNREF
        return null;
    }
    maxlatency = member->maxlatency; // Récupération de la latence maximale admissible
    member_unlock(member); /// UNLOCK
    for (;;) { // Sélection du paquet
#endif
        skb = connection_pop(connection); // Pop de premier paquet (s'il existe)
        if (unlikely(!skb)) {
            connection_unlock(connection); /// UNLOCK
            connection_unref(connection); /// UNREF
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
            member_unref(member); /// UNREF
#endif
            return null;
        }
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
        if (maxlatency != 0 && connection_isip(skb) && connection->protocol == IPPROTO_UDP) { // Limitation de latence pour l'UDP/IP
            nint deltatime = (nint) (((nint64) (((union ktime) ktime_get_real()).tv64) - (nint64) (((union ktime) skb_get_ktime(skb)).tv64)) / 1000); // Delta temps envoi-réception (en µs)
            if (deltatime > maxlatency) { // Drop du packet
                consume_skb(skb);
                continue; // Sélection du paquet suivant
            }
        }
        break;
    }
#endif
    { // Mise à jour des statistiques
        zint deltatime = (zint) (((zint64) (((union ktime) ktime_get_real()).tv64) - (zint64) (((union ktime) skb_get_ktime(skb)).tv64)) / 1000); // Delta temps envoi-réception (en µs)
        zint size = (zint) skb->truesize; // Taille des données envoyées
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY != 1
        struct member* member;
        member = connection->member;
        member_ref(member); /// REF
#endif
        connection->lastsize = (nint) size;
        throughput_add(&(connection->throughup), size);
        if (unlikely(!connection_schedule(connection))) { // Rescheduling de la connexion, UNLOCK
            member_unref(member); /// UNREF
            connection_unref(connection); /// UNREF
            return skb;
        }
        connection_unref(connection); /// UNREF
        if (unlikely(!member_lock(member))) { /// LOCK
            member_unref(member); /// UNREF
            return skb;
        }
        throughput_add(&(member->throughup), size); // Compte du débit montant
        average_add(&(member->latency), deltatime); // Compte de la latence
        member_unlock(member); /// UNLOCK
        member_unref(member); /// UNREF
        if (unlikely(!router_lock(router))) /// LOCK
            return skb;
        throughput_add(&(router->throughup), size); // Compte du débit montant
        average_add(&(router->latency), deltatime); // Compte de la latence
        router_unlock(router); /// UNLOCK
    }
    return skb;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Interface avec les hooks ▔
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
