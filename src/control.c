/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▁ Documentation ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/**
 * @file   control.c
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
 * Interface le module entre la partie scheduling et l'administrateur.
**/

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Documentation ▔
/// ▁ Déclarations ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Headers externes
#include <linux/time.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/netdevice.h>

/// Headers internes
#include "control.h"

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Messages d'erreur
#define CONTROL_INVALIDSHOW   "Invalid show on attribute " NINT  // L'attribut ne devait pas être lu
#define CONTROL_INVALIDSTORE  "Invalid store on attribute " NINT // L'attribut ne devait pas être écrit
#define CONTROL_IDALREADYUSED "ID " NINT " already used"         // L'identifiant est déjà utilisé

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Prototypes
static inline void control_lock(void);
static inline void control_unlock(void);
static struct object_router* control_getbyid_router(nint);
static struct object_member* control_getbyid_member(nint);

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Déclarations ▔
/// ▁ Helpers d'accès ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Obtient l'identifiant de l'attribut.
 * @param attr Structure attribut
 * @return Identifiant de l'attribut
**/
static inline nint control_getidbyattr(struct attribute* attr) {
    return container_of(attr, struct control_attribute, attribute)->id;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Obtient l'objet par le kobject.
 * @param kobject Pointeur sur le kobject
 * @return Objet
**/
static inline struct object_router* control_getobjectrouterbykobject(struct kobject* kobject) {
    return container_of(kobject, struct object_router, kobject);
}
static inline struct object_member* control_getobjectmemberbykobject(struct kobject* kobject) {
    return container_of(kobject, struct object_member, kobject);
}
static inline struct object_scheduler* control_getobjectschedulerbykobject(struct kobject* kobject) {
    return container_of(kobject, struct object_scheduler, kobject);
}

/** Obtient la structure par le kobject.
 * @param kobject Pointeur sur le kobject
 * @return Structure
**/
static inline struct router* control_getrouterbykobject(struct kobject* kobject) {
    return &(control_getobjectrouterbykobject(kobject)->structure);
}
static inline struct member* control_getmemberbykobject(struct kobject* kobject) {
    return &(control_getobjectmemberbykobject(kobject)->structure);
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Helpers d'accès ▔
/// ▁ Log des connexions ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

#if FAIRCONF_CONNLOG == 1

/// Format du début d'une entrée de log
#define CONNLOG_OUTFORMATBASE NINT "\t" NINT "\t" NINT "\t" NINT "\t"

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Verrouillage du log de connexions.
 * @param connlog Structure du log de connexions
**/
static inline void connlog_lock(struct connlog* connlog) {
    spin_lock(&(connlog->lock));
}

/** Déverrouillage du log de connexions.
 * @param connlog Structure du log de connexions
**/
static inline void connlog_unlock(struct connlog* connlog) {
    spin_unlock(&(connlog->lock));
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Initialise les logs des connexions.
 * @param connlog Structure des logs des connexions
 * @return Précise si l'opération est un succès
**/
static bool connlog_init(struct connlog* connlog) {
    connlog->buffer = (nint8*) kmalloc(CONNLOG_BUFFERSIZE * sizeof(nint8), GFP_KERNEL); // Allocation du buffer
    if (unlikely(!connlog->buffer)) { // Échec d'allocation
        return false;
    }
    connlog->slab = kmem_cache_create("faircrave_connlog", sizeof(struct connlog_entry), 0, 0, null); // Allocation du slab
    if (unlikely(!connlog->slab)) { // Échec de création
        kfree(connlog->buffer); // Libération du buffer de sortie
        return false;
    }
    connlog->size = 0; // Buffer non utilisé
    aint_set(&(connlog->count), 0); // Aucune entrée
    aint_set(&(connlog->limit), CONNLOG_LOGENTRIES); // Limite par défaut
    INIT_LIST_HEAD(&(connlog->list));
    spin_lock_init(&(connlog->lock));
    return true;
}

/** Nettoie les logs des connexions.
 * @param connlog Structure des logs des connexions
 * @return Précise si l'opération est un succès
**/
static bool connlog_clean(struct connlog* connlog) {
    connlog_lock(connlog); /// LOCK
    kfree(connlog->buffer); // Libération
    { // Flush des entrées
        struct connlog_entry* entry; // Entrée en cours
        struct connlog_entry* next;  // Entrée suivante
        list_for_each_entry_safe(entry, next, &(connlog->list), list)
            kmem_cache_free(connlog->slab, entry); // Libération de l'entrée
        INIT_LIST_HEAD(&(connlog->list)); // Liste désormais vide
    }
    kmem_cache_destroy(connlog->slab); // Destruction du slab
    connlog_unlock(connlog); /// UNLOCK
    return true;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Obtient le nombre limite d'entrées.
 * @param connlog Structure de log des connexions
 * @return Nombre limite d'entrées
**/
static inline nint connlog_getlimit(struct connlog* connlog) {
    return aint_read(&(connlog->limit)); // Lecture de la limite
}

/** Affecte le nombre limite d'entrées.
 * @param connlog Structure de log des connexions
 * @param limit   Nombre limite d'entrées
**/
static inline void connlog_setlimit(struct connlog* connlog, nint limit) {
    aint_set(&(connlog->limit), limit); // Affectation de la limite
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Ajoute une connexion dans la liste.
 * @param connlog  Structure de log des connexions
 * @param member   Identifiant de l'adhérent
 * @param router   Identifiant du routeur
 * @param duration Durée de la connexion (en secondes)
 * @param protocol Identifiant du protocol, détermine la taille de l'adresse cible
 * @param address  Adresse IPv4/v6 cible
 * @param port     Port cible
 * @return Précise si l'opération est un succès
**/
bool connlog_push(struct connlog* connlog, nint member, nint router, nint duration, nint16 protocol, nint8* address, nint16 port) {
    struct connlog_entry* entry; // Entrée de log de connexion
    connlog_lock(connlog); /// LOCK
    if (unlikely(!connlog->buffer)) { // Non alloué
        connlog_unlock(connlog); /// UNLOCK
        return false;
    } else if (aint_read(&(connlog->count)) >= aint_read(&(connlog->limit))) { // Dépassement de la limite
        connlog_unlock(connlog); /// UNLOCK
        return false;
    } else { // Allocation de la structure
        entry = kmem_cache_alloc(connlog->slab, GFP_KERNEL); // Allocation de la structure
        if (unlikely(!entry)) { // Échec d'allocation
            connlog_unlock(connlog); /// UNLOCK
            return false;
        }
    }
    { // Initialisation de la structure
        INIT_LIST_HEAD(&(entry->list));
        entry->member = member;
        entry->router = router;
        entry->date = (nint) get_seconds() - duration;
        entry->duration = duration;
        entry->target.protocol = protocol;
        entry->target.port = port;
        if (protocol == CONNLOG_PROTO_TCPIPv4 || protocol == CONNLOG_PROTO_UDPIPv4) { // Sur IPv4
            memcpy(entry->target.address.ipv4, address, IPV4_SIZE); // Copie de l'adresse
        } else if (protocol == CONNLOG_PROTO_TCPIPv6 || protocol == CONNLOG_PROTO_UDPIPv6) { // Sur IPv6
            memcpy(entry->target.address.ipv6, address, IPV6_SIZE); // Copie de l'adresse
        } else { // Protocole inconnu
            log(KERN_ERR, "Unknow protocol %u", protocol);
            connlog_unlock(connlog); /// UNLOCK
            return false;
        }
    }
    list_add_tail(&(entry->list), &(connlog->list)); // Ajout à la fin de la liste
    connlog_unlock(connlog); /// UNLOCK
    aint_inc(&(connlog->count)); // Plus une entrée
    return true;
}

/** Prépare la chaîne de caractère représentant la plus ancienne entrée.
 * @param connlog  Structure de log des connexions
 * @return Taille de la chaîne mise en cache (0 si aucune)
**/
static nint connlog_cache(struct connlog* connlog) {
    nint size; // Taille retournée
    connlog_lock(connlog); /// LOCK
    if (unlikely(!connlog->buffer)) { // Structure nettoyée
        connlog_unlock(connlog); /// UNLOCK
        return 0;
    }
    if (unlikely(connlog->size != 0)) { // Chaîne déjà construite
        size = connlog->size;
    } else if (list_empty(&(connlog->list))) { // Aucune entrée
        size = 0;
    } else { // Construction de la chaîne
        struct connlog_entry* entry = container_of(connlog->list.next, struct connlog_entry, list); // Plus ancienne entrée
        switch (entry->target.protocol) { // Format selon le protocole
            case CONNLOG_PROTO_TCPIPv4: {
                size = sprintf(connlog->buffer, CONNLOG_OUTFORMATBASE "tcp4\t", entry->member, entry->router, entry->date, entry->duration); // Avant l'adresse
                size += tools_ipv4tostr((nint8*) (connlog->buffer + size), entry->target.address.ipv4); // Adresse IP
                size += sprintf(connlog->buffer + size, "\t%u\n", entry->target.port); // Après l'adresse
            } break;
            case CONNLOG_PROTO_UDPIPv4: {
                size = sprintf(connlog->buffer, CONNLOG_OUTFORMATBASE "udp4\t", entry->member, entry->router, entry->date, entry->duration); // Avant l'adresse
                size += tools_ipv4tostr((nint8*) (connlog->buffer + size), entry->target.address.ipv4); // Adresse IP
                size += sprintf(connlog->buffer + size, "\t%u\n", entry->target.port); // Après l'adresse
            } break;
            case CONNLOG_PROTO_TCPIPv6: {
                size = sprintf(connlog->buffer, CONNLOG_OUTFORMATBASE "tcp6\t", entry->member, entry->router, entry->date, entry->duration); // Avant l'adresse
                size += tools_ipv6tostr((nint8*) (connlog->buffer + size), (nint16*) entry->target.address.ipv6); // Adresse IP
                size += sprintf(connlog->buffer + size, "\t%u\n", entry->target.port); // Après l'adresse
            } break;
            case CONNLOG_PROTO_UDPIPv6: {
                size = sprintf(connlog->buffer, CONNLOG_OUTFORMATBASE "udp6\t", entry->member, entry->router, entry->date, entry->duration); // Avant l'adresse
                size += tools_ipv6tostr((nint8*) (connlog->buffer + size), (nint16*) entry->target.address.ipv6); // Adresse IP
                size += sprintf(connlog->buffer + size, "\t%u\n", entry->target.port); // Après l'adresse
            } break;
            default: // Protocole inconnu
                size = 0;
                log(KERN_ERR, "Unknow protocol %u", entry->target.protocol);
                break;
        }
    }
    connlog_unlock(connlog); /// UNLOCK
    return size;
}

/** Supprime la plus ancienne entrée.
 * @param connlog Structure de log des connexions
**/
static void connlog_pop(struct connlog* connlog) {
    connlog_lock(connlog); /// LOCK
    if (unlikely(!connlog->buffer)) { // Structure nettoyée
        log(KERN_ERR, "Pop from a cleaned structure");
        connlog_unlock(connlog); /// UNLOCK
        return;
    }
    connlog->size = 0; // Buffer "vidé"
    if (!list_empty(&(connlog->list))) { // Au moins une entrée
        struct connlog_entry* entry = container_of(connlog->list.next, struct connlog_entry, list); // Première entrée
        list_del(&(entry->list)); // Suppression de l'entrée
        kmem_cache_free(connlog->slab, entry); // Libération de la structure
    }
    connlog_unlock(connlog); /// UNLOCK
    aint_dec(&(connlog->count)); // Moins une entrée
}

#endif

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Log des connexions ▔
/// ▁ Objet routeur ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Prototype
static void object_router_release(struct kobject*);
static ssize_t object_router_show(struct kobject*, struct attribute*, char*);
static ssize_t object_router_store(struct kobject*, struct attribute*, const char*, size_t);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Liste des attributs
static struct control_attribute object_router_attribute[] = {
    { // STATUS
        .id = ID_ROUTER_STATUS,
        .attribute = {
            .name = LABEL_ROUTER_STATUS,
            .mode = 0666
        }
    },
    { // NETDEVICE
        .id = ID_ROUTER_NETDEVICE,
        .attribute = {
            .name = LABEL_ROUTER_NETDEVICE,
            .mode = 0666
        }
    },
    { // LATENCY
        .id = ID_ROUTER_LATENCY,
        .attribute = {
            .name = LABEL_ROUTER_LATENCY,
            .mode = 0444
        }
    },
    { // ALLOWIPV4
        .id = ID_ROUTER_ALLOWIPV4,
        .attribute = {
            .name = LABEL_ROUTER_ALLOWIPV4,
            .mode = 0666
        }
    },
    { // ALLOWIPV6
        .id = ID_ROUTER_ALLOWIPV6,
        .attribute = {
            .name = LABEL_ROUTER_ALLOWIPV6,
            .mode = 0666
        }
    },
    { // TPUTLIMIT
        .id = ID_ROUTER_TPUTLIMIT,
        .attribute = {
            .name = LABEL_ROUTER_TPUTLIMIT,
            .mode = 0666
        }
    },
    { // TPUTUP
        .id = ID_ROUTER_TPUTUP,
        .attribute = {
            .name = LABEL_ROUTER_TPUTUP,
            .mode = 0444
        }
    },
    { // TPUTDOWN
        .id = ID_ROUTER_TPUTDOWN,
        .attribute = {
            .name = LABEL_ROUTER_TPUTDOWN,
            .mode = 0444
        }
    }
};

/// Table des attributs
static struct attribute* object_router_attributes[] = {
    &(object_router_attribute[0].attribute),
    &(object_router_attribute[1].attribute),
    &(object_router_attribute[2].attribute),
    &(object_router_attribute[3].attribute),
    &(object_router_attribute[4].attribute),
    &(object_router_attribute[5].attribute),
    &(object_router_attribute[6].attribute),
    &(object_router_attribute[7].attribute),
    null // Toujours terminée par null
};

/// Définition des opérations
static struct sysfs_ops object_router_ops = {
    .show = object_router_show,
    .store = object_router_store
};

/// Définition du type
static struct kobj_type object_router_type = {
    .release = object_router_release,
    .sysfs_ops = &object_router_ops,
    .default_attrs = object_router_attributes,
    .child_ns_type = null,
    .namespace = null
};

/// Statuts fonction de l'identifiant (/!\ ordre fixe)
static const char* const object_router_status[] = {
    "online",
    "offline",
    "closing"
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur création du routeur.
 * @param id Identifiant du routeur
 * @return Précise si l'opération est un succès
**/
static bool object_router_create(nint id) {
    if (control_getbyid_router(id)) { // Identifiant déjà utilisé
        log(KERN_WARNING, CONTROL_IDALREADYUSED, id);
        return false;
    }
    { // Création de l'objet
        struct object_router* object_router; // Objet routeur
        if (unlikely(!(object_router = (struct object_router*) kzalloc(sizeof(struct object_router), GFP_KERNEL)))) // Échec d'allocation de l'objet
            goto ERR_1;
        if (unlikely(kobject_init_and_add(&(object_router->kobject), &object_router_type, control.routers, NINT, id))) // Échec d'initialisation ou ajout
            goto ERR_2;
        if (unlikely(!router_init(&(object_router->structure)))) // Initialisation de la structure, référencée
            goto ERR_3;
        object_router->id = id; // Inscription de l'identifiant
        list_add(&(object_router->list), &(control.list_object_router)); // Enregistrement du routeur, prise de référence
        return true;
        ERR_3: kobject_put(control.members); // Libération du kobject
        ERR_2: kfree(object_router); // Libération de l'objet
        ERR_1: return false;
    }
}

/** Sur libération du routeur, destructeur du kobject.
 * @param kobject Pointeur sur l'objet routeur
**/
static void object_router_release(struct kobject* kobject) {
    struct object_router* object_router = control_getobjectrouterbykobject(kobject);
    list_del(&(object_router->list)); // Suppression de l'objet
    router_clean(&(object_router->structure)); // Nettoyage de la structure
    router_unref(&(object_router->structure)); /// UNREF
}

/** Sur lecture des données.
 * @param kobject Pointeur sur le kobject concerné
 * @param attr    Structure d'attribut
 * @param data    Buffer de données reçues
 * @param size    Taille du buffer
 * @return Nombre d'octets lus
**/
static ssize_t object_router_show(struct kobject* kobject, struct attribute* attr, char* data) {
    ssize_t object_router_do_show(nint id) {
        switch (id) {
            case ID_ROUTER_STATUS: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                const char* value; // Chaîne en sortie
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                if (router->closing) { // Online mais en cours de fermeture
                    if (sortlist_empty(&(router->connections.sortlist))) { // Plus aucune connexion en cours
                        value = "closed";
                    } else {
                        value = object_router_status[2];
                    }
                } else if (!router->reachable) { // Non atteignable
                    value = "unreachable";
                } else if (!router->online) { // Offline
                    value = object_router_status[1];
                } else { // Online
                    value = object_router_status[0];
                }
                router_unlock(router); /// UNLOCK
                return sprintf(data, "%s\n", value);
            }
            case ID_ROUTER_NETDEVICE: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                struct netdev* netdev;
                nint length; // Longueur de la chaîne
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                netdev = router->netdev.ptr;
                if (!netdev) { // Non existant
                    router_unlock(router); /// UNLOCK
                    return 0;
                }
                netdev_ref(netdev); /// REF
                router_unlock(router); /// UNLOCK
                if (unlikely(!netdev_lock(netdev))) { /// LOCK
                    netdev_unref(netdev); /// UNREF
                    return 0;
                }
                length = strlen(strcpy(data, netdev->netdev->name)); // Copie et récupération de la taille
                if (likely(length < PAGE_SIZE)) // Encore de la place pour ajouter...
                    data[length++] = '\n'; // ...une nouvelle ligne
                netdev_unlock(netdev); /// UNLOCK
                netdev_unref(netdev); /// UNREF
                return length;
            }
            case ID_ROUTER_LATENCY: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                zint value; // Valeur
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                value = average_get(&(router->latency));
                router_unlock(router); /// UNLOCK
                return sprintf(data, NINT "\n", value);
            }
            case ID_ROUTER_ALLOWIPV4: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                bool value; // Valeur
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                value = router->allowipv4;
                router_unlock(router); /// UNLOCK
                return sprintf(data, "%u\n", (value ? 1 : 0));
            }
            case ID_ROUTER_ALLOWIPV6: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                bool value; // Valeur
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                value = router->allowipv6;
                router_unlock(router); /// UNLOCK
                return sprintf(data, "%u\n", (value ? 1 : 0));
            }
            case ID_ROUTER_TPUTLIMIT: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                nint value; // Valeur
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                value = router->throughlimit;
                router_unlock(router); /// UNLOCK
                return sprintf(data, NINT "\n", value / 1000); // Conversion o/s -> ko/s
            }
            case ID_ROUTER_TPUTUP: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                zint value; // Valeur
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                value = throughput_get(&(router->throughup)) * HZ;
                router_unlock(router); /// UNLOCK
                return sprintf(data, NINT "\n", value / 1000); // Conversion o/s -> ko/s
            }
            case ID_ROUTER_TPUTDOWN: {
                struct router* router = control_getrouterbykobject(kobject); // Structure du routeur
                zint value; // Valeur
                if (unlikely(!router_lock(router))) /// LOCK
                    return 0;
                value = throughput_get(&(router->throughdown)) * HZ;
                router_unlock(router); /// UNLOCK
                return sprintf(data, ZINT "\n", value / 1000); // Conversion o/s -> ko/s
            }
            default: // Attribut inconnu
                log(KERN_ERR, CONTROL_INVALIDSHOW, id);
                return 0;
        }
    }
    ssize_t size; // Taille écrite
    control_lock(); /// LOCK
    size = object_router_do_show(control_getidbyattr(attr));
    control_unlock(); /// UNLOCK
    return size;
}

/** Sur écriture de données.
 * @param kobject Pointeur sur le kobject concerné
 * @param attr    Structure d'attribut
 * @param data    Buffer de données reçues
 * @param size    Taille du buffer
 * @return Nombre d'octets écrits
**/
static ssize_t object_router_store(struct kobject* kobject, struct attribute* attr, const char* data, size_t size) {
    void object_router_do_store(nint id) {
        switch (id) {
            case ID_ROUTER_STATUS: {
                nint order = (1 << sizeof(object_router_status) / sizeof(const char*)) - 1; // Ordres passés (à ce stade, tous les ordres sont possibles)
                { // Recherche de l'ordre
                    nint8 car;
                    nint i, j; // Compteurs
                    for (i = 0; i < size && order > 0; i++) {
                        car = data[i];
                        if (car == '\n') // Fin de traitement
                            break;
                        for (j = 0; j < sizeof(object_router_status) / sizeof(const char*) && order > 0; j++) { // Pour chaque ordre (s'il en reste)
                            if ((order & (1 << j)) && car != object_router_status[j][i]) // Ne correspond plus
                                order &= ~(1 << j); // Élimination de l'ordre
                        }
                    }
                }
                switch (order) { // Traitement de l'ordre
                    case 1: // online
                        if (!router_setonline(control_getrouterbykobject(kobject), true)) // Passage en online
                            log(KERN_ERR, "Unable to set router " NINT " online", control_getobjectrouterbykobject(kobject)->id);
                        break;
                    case 2: // offline
                        if (!router_setonline(control_getrouterbykobject(kobject), false)) // Passage en offline
                            log(KERN_ERR, "Unable to set router " NINT " offline", control_getobjectrouterbykobject(kobject)->id);
                        break;
                    case 4: // closing
                        router_end(control_getrouterbykobject(kobject)); // Mise en fermeture d'un routeur
                        break;
                    default:
                        log(KERN_ERR, "Invalid order");
                        break;
                }
            } return;
            case ID_ROUTER_NETDEVICE: {
                struct router* router; // Structure du routeur
                struct net_device* net_device;
                struct netdev* netdev;
                char* name; // Nom de la passerelle
                if (size == 0 || *data == '\n') { // Suppression de la network device
                    router_setnetdev(control_getrouterbykobject(kobject), null); // Suppression
                    return;
                }
                name = kmalloc((size + 1) * sizeof(char), GFP_KERNEL); // Allocation de la mémoire pour le nom
                memcpy(name, data, size); // Copie du nom
                if (name[size - 1] == '\n') { // Écrasement
                    name[size - 1] = '\0';
                } else { // Ajout
                    name[size] = '\0';
                }
                net_device = dev_get_by_name(&init_net, name); // Récupération de la network device
                if (!net_device) { // Non trouvée
                    kfree(name); // Libération de la mémoire
                    log(KERN_ERR, "Unknow network device %s", name);
                    return;
                }
                kfree(name); // Libération de la mémoire
                router = control_getrouterbykobject(kobject);
                netdev = scheduler_getnetdev(net_device); // Récupération de la netdev, référencée
                if (!netdev || !router_setnetdev(router, netdev)) { // Non récupérée ou non appliqué
                    if (netdev) // Trouvé
                        netdev_unref(netdev); /// UNREF
                    dev_put(net_device); // Décompte d'une référence
                    log(KERN_ERR, "Unable to set netdevice for router " NINT, control_getobjectrouterbykobject(kobject)->id);
                    return;
                }
                netdev_unref(netdev); /// UNREF
                dev_put(net_device); // Décompte d'une référence
            } return;
            case ID_ROUTER_ALLOWIPV4: {
                nint value;
                if (!tools_strtonint((nint8*) data, size, &value)) // Conversion base 10
                    return;
                value = (value == 1 ? ROUTER_YES : ROUTER_NO);
                router_allowip(control_getrouterbykobject(kobject), value, ROUTER_UNCHANGE); // Modification IPv4
            } return;
            case ID_ROUTER_ALLOWIPV6: {
                nint value;
                if (!tools_strtonint((nint8*) data, size, &value)) // Conversion base 10
                    return;
                value = (value == 1 ? ROUTER_YES : ROUTER_NO);
                router_allowip(control_getrouterbykobject(kobject), ROUTER_UNCHANGE, value); // Modification IPv6
            } return;
            case ID_ROUTER_TPUTLIMIT: {
                nint throughlimit;
                zint delta; // Différence de débit
                struct router* router;
                if (!tools_strtonint((nint8*) data, size, &throughlimit)) // Conversion base 10
                    return;
                router = control_getrouterbykobject(kobject); // Structure du routeur
                if (unlikely(!router_lock(router))) /// LOCK
                    return;
                delta = (zint) throughlimit * 1000 - router->throughlimit; // Différence de débit (en o/s)
                router->throughlimit = throughlimit * 1000; // Conversion ko/s -> o/s
                router_unlock(router); /// UNLOCK
                if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
                    return;
                scheduler.throughput += delta; // Application de la différence
                scheduler_unlock(&scheduler); /// UNLOCK
            } return;
            default: // Attribut inconnu
                log(KERN_ERR, CONTROL_INVALIDSTORE, id);
                return;
        }
    }
    control_lock(); /// LOCK
    object_router_do_store(control_getidbyattr(attr));
    control_unlock(); /// UNLOCK
    return size;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Objet routeur ▔
/// ▁ Objet adhérent ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Prototype
static void object_member_release(struct kobject*);
static ssize_t object_member_show(struct kobject*, struct attribute*, char*);
static ssize_t object_member_store(struct kobject*, struct attribute*, const char*, size_t);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Liste des attributs
static struct control_attribute object_member_attribute[] = {
    { // TRIPLE
        .id = ID_MEMBER_TUPLES,
        .attribute = {
            .name = LABEL_MEMBER_TUPLES,
            .mode = 0666
        }
    },
    { // ROUTER
        .id = ID_MEMBER_ROUTER,
        .attribute = {
            .name = LABEL_MEMBER_ROUTER,
            .mode = 0666
        }
    },
    { // LATENCY
        .id = ID_MEMBER_LATENCY,
        .attribute = {
            .name = LABEL_MEMBER_LATENCY,
            .mode = 0444
        }
    },
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
    { // MAXLATENCY
        .id = ID_MEMBER_MAXLATENCY,
        .attribute = {
            .name = LABEL_MEMBER_MAXLATENCY,
            .mode = 0666
        }
    },
#endif
    { // PRIORITY
        .id = ID_MEMBER_PRIORITY,
        .attribute = {
            .name = LABEL_MEMBER_PRIORITY,
            .mode = 0666
        }
    },
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
    { // TPUTASK
        .id = ID_MEMBER_TPUTASK,
        .attribute = {
            .name = LABEL_MEMBER_TPUTASK,
            .mode = 0444
        }
    },
    { // TPUTLOST
        .id = ID_MEMBER_TPUTLOST,
        .attribute = {
            .name = LABEL_MEMBER_TPUTLOST,
            .mode = 0444
        }
    },
#endif
    { // TPUTUP
        .id = ID_MEMBER_TPUTUP,
        .attribute = {
            .name = LABEL_MEMBER_TPUTUP,
            .mode = 0444
        }
    },
    { // TPUTDOWN
        .id = ID_MEMBER_TPUTDOWN,
        .attribute = {
            .name = LABEL_MEMBER_TPUTDOWN,
            .mode = 0444
        }
    }
};

/// Table des attributs
static struct attribute* object_member_attributes[] = {
    &(object_member_attribute[0].attribute),
    &(object_member_attribute[1].attribute),
    &(object_member_attribute[2].attribute),
    &(object_member_attribute[3].attribute),
    &(object_member_attribute[4].attribute),
    &(object_member_attribute[5].attribute),
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
    &(object_member_attribute[6].attribute),
    &(object_member_attribute[7].attribute),
    &(object_member_attribute[8].attribute),
#else
    &(object_member_attribute[6].attribute),
#endif
#else
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
    &(object_member_attribute[6].attribute),
    &(object_member_attribute[7].attribute),
#endif
#endif
    null // Toujours terminée par null
};

/// Définition des opérations
static struct sysfs_ops object_member_ops = {
    .show = object_member_show,
    .store = object_member_store
};

/// Définition du type
static struct kobj_type object_member_type = {
    .release = object_member_release,
    .sysfs_ops = &object_member_ops,
    .default_attrs = object_member_attributes,
    .child_ns_type = null,
    .namespace = null
};

/// Ordres pour les tuples (/!\ l'ordre ne doit pas être changé - voir store/tuple)
static const char* object_member_tuple_orders[] = {
    "add4",
    "add6",
    "del4",
    "del6"
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur création de l'adhérent.
 * @param id Identifiant de l'adhérent
 * @return Précise si l'opération est un succès
**/
static bool object_member_create(nint id) {
    if (control_getbyid_member(id)) { // Identifiant déjà utilisé
        log(KERN_WARNING, CONTROL_IDALREADYUSED, id);
        return false;
    }
    { // Création de l'objet
        struct object_member* object_member; // Objet adhérent
        if (unlikely(!(object_member = (struct object_member*) kzalloc(sizeof(struct object_member), GFP_KERNEL)))) // Échec d'allocation de l'objet
            goto ERR_1;
        if (unlikely(kobject_init_and_add(&(object_member->kobject), &object_member_type, control.members, NINT, id))) // Échec d'initialisation ou ajout
            goto ERR_2;
        member_init(&(object_member->structure)); // Initialisation de l'adhérent, référencé
        object_member->id = id; // Inscription de l'identifiant
        list_add(&(object_member->list), &(control.list_object_member)); // Enregistrement de l'adhérent, prise de référence
        return true;
        ERR_2: kfree(object_member); // Libération de l'objet
        ERR_1: return false;
    }
}

/** Sur libération de l'adhérent, destructeur du kobject.
 * @param kobject Pointeur sur l'objet adhérent
**/
static void object_member_release(struct kobject* kobject) {
    struct object_member* object_member = control_getobjectmemberbykobject(kobject);
    list_del(&(object_member->list)); // Prise de référence
    member_clean(&(object_member->structure)); // Nettoyage de la structure
    member_unref(&(object_member->structure)); /// UNREF
}

/** Sur lecture des données.
 * @param kobject Pointeur sur le kobject concerné
 * @param attr    Structure d'attribut
 * @param data    Buffer de données à écrire
 * @return Nombre d'octets lus
**/
static ssize_t object_member_show(struct kobject* kobject, struct attribute* attr, char* data) {
    ssize_t object_member_do_show(nint id) {
        switch (id) {
            case ID_MEMBER_TUPLES: {
                struct member* member = control_getmemberbykobject(kobject); // Structure du membre
                nint size = PAGE_SIZE; // Taille écrite
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                { // Print des tuples
                    struct tuple* tuple; // Tuple en cours (/!\ verrouillage inutile : appelé depuis le contrôle qui est sérialisé et seul pouvant les modifier)
                    list_for_each_entry(tuple, &(member->tuples), list) { // Pour tous les tuples
                        member_unlock(member); /// UNLOCK
                        if (tuple->address.version == 4) { // IPv4
                            nint length; // Espace occupée par l'IP
                            if (size < 2 + MAC_TEXTSIZE + IPV4_TEXTSIZE) // Plus assez de place
                                break;
                            tools_mactostr(data, tuple->address.mac);
                            data += MAC_TEXTSIZE;
                            *(data++) = ' ';
                            length = tools_ipv4tostr(data, tuple->address.ipv4);
                            data += length;
                            *(data++) = '\n';
                            size -= MAC_TEXTSIZE + length + 2; // Décompte de l'espace occupé réellement
                        } else { // IPv6
                            nint length; // Espace occupée par l'IP
                            if (size < 2 + MAC_TEXTSIZE + IPV6_TEXTSIZE) // Plus assez de place
                                break;
                            tools_mactostr(data, tuple->address.mac);
                            data += MAC_TEXTSIZE;
                            *(data++) = ' ';
                            length = tools_ipv6tostr(data, (nint16*) tuple->address.ipv6);
                            data += length;
                            *(data++) = '\n';
                            size -= MAC_TEXTSIZE + length + 2; // Décompte de l'espace occupé réellement
                        }
                        if (unlikely(!member_lock(member))) /// LOCK
                            return 0;
                    }
                }
                member_unlock(member); /// UNLOCK
                return PAGE_SIZE - size;
            }
            case ID_MEMBER_ROUTER: {
                ssize_t size; // Taille écrite
                struct router* router = member_getrouter(control_getmemberbykobject(kobject));
                if (router) { // Possède un routeur préférée
                    size = sprintf(data, NINT "\n", control_getobjectrouterbystructure(router)->id); // Sortie de l'identifiant
                } else {
                    size = 0;
                }
                return size;
            }
            case ID_MEMBER_LATENCY: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                zint value; // Valeur
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                value = average_get(&(member->latency));
                member_unlock(member); /// UNLOCK
                return sprintf(data, ZINT "\n", value);
            }
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
            case ID_MEMBER_MAXLATENCY: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                nint value; // Valeur
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                value = member->maxlatency;
                member_unlock(member); /// UNLOCK
                return sprintf(data, NINT "\n", value);
            }
#endif
            case ID_MEMBER_PRIORITY: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                nint value; // Valeur
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                value = member->priority;
                member_unlock(member); /// UNLOCK
                return sprintf(data, NINT "\n", value);
            }
#if FAIRCONF_SCHEDULER_MOREMEMBERSTATS == 1
            case ID_MEMBER_TPUTASK: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                zint value; // Valeur
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                value = throughput_get(&(member->throughask)) * HZ;
                member_unlock(member); /// UNLOCK
                return sprintf(data, ZINT "\n", value / 1000); // Conversion o/s -> ko/s
            }
            case ID_MEMBER_TPUTLOST: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                zint value; // Valeur
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                value = throughput_get(&(member->throughlost)) * HZ;
                member_unlock(member); /// UNLOCK
                return sprintf(data, ZINT "\n", value / 1000); // Conversion o/s -> ko/s
            }
#endif
            case ID_MEMBER_TPUTUP: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                zint value; // Valeur
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                value = throughput_get(&(member->throughup)) * HZ;
                member_unlock(member); /// UNLOCK
                return sprintf(data, ZINT "\n", value / 1000); // Conversion o/s -> ko/s
            }
            case ID_MEMBER_TPUTDOWN: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                zint value; // Valeur
                if (unlikely(!member_lock(member))) /// LOCK
                    return 0;
                value = throughput_get(&(member->throughdown)) * HZ;
                member_unlock(member); /// UNLOCK
                return sprintf(data, ZINT "\n", value / 1000); // Conversion o/s -> ko/s
            }
            default: // Attribut inconnu
                log(KERN_ERR, CONTROL_INVALIDSHOW, id);
                return 0;
        }
    }
    ssize_t size; // Taille écrite
    control_lock(); /// LOCK
    size = object_member_do_show(control_getidbyattr(attr));
    control_unlock(); /// UNLOCK
    return size;
}

/** Sur écriture de données.
 * @param kobject Pointeur sur le kobject concerné
 * @param attr    Structure d'attribut
 * @param data    Buffer de données reçues
 * @param size    Taille du buffer
 * @return Nombre d'octets écrits
**/
static ssize_t object_member_store(struct kobject* kobject, struct attribute* attr, const char* data, size_t size) {
    void object_member_do_store(nint id) {
        switch (id) {
            case ID_MEMBER_TUPLES: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent, considéré référencé
                nint8* line; // Ligne en cours, modifiable en cours de traitement
                nint linesize; // Taille de la ligne, modifiable en cours de traitement
                nint pos = 0; // Position
                while (pos < size) { // Pour toutes les lignes
                    line = ((nint8*) data) + pos;
                    linesize = 0;
                    while (data[pos] != '\n' && pos < size) { // Recherche de fin de ligne
                        linesize++;
                        pos++;
                    }
                    pos++; // On saute la fin de ligne
                    if (linesize > 0) { // Traitement
                        nint order = (1 << sizeof(object_member_tuple_orders) / sizeof(const char*)) - 1; // Ordres passés (à ce stade, tous les ordres sont possibles)
                        nint version; // Version IP
                        bool add; // Ajout ou suppression
                        { // Recherche de l'ordre
                            nint8 car;
                            nint i, j; // Compteurs
                            for (i = 0; i < linesize && order > 0; i++) {
                                car = line[i];
                                if (car == ' ') { // Fin de traitement
                                    line += i + 1; // Début adresse MAC (si saisie valide)
                                    linesize -= i + 1; // Décompte de l'ordre et de l'espace
                                    break;
                                }
                                for (j = 0; j < sizeof(object_member_tuple_orders) / sizeof(const char*) && order > 0; j++) { // Pour chaque ordre (s'il en reste)
                                    if ((order & (1 << j)) && car != object_member_tuple_orders[j][i]) // Ne correspond plus
                                        order &= ~(1 << j); // Élimination de l'ordre
                                }
                            }
                        }
                        switch (order) { // Traitement de l'ordre
                            case 1:
                                version = 4;
                                add = true;
                                break;
                            case 2:
                                version = 6;
                                add = true;
                                break;
                            case 4:
                                version = 4;
                                add = false;
                                break;
                            case 8:
                                version = 6;
                                add = false;
                                break;
                            default:
                                log(KERN_ERR, "Invalid order");
                                goto NEXTLINE;
                        }
                        { // Exécution de l'ordre
                            nint8 mac[MAC_SIZE]; // Adresse MAC
                            nint8 ip[IPV6_SIZE]; // Adresse IPv4/IPv6
                            { // Récupération et conversion des adresses
                                nint8* textmac; // Adresse mac
                                nint8* textip; // Adresse IPvX
                                if (linesize < MAC_TEXTSIZE + 1) { // Manque adresse MAC
                                    log(KERN_ERR, "Malformed input");
                                    goto NEXTLINE;
                                }
                                textmac = line; // Adresse MAC sur MAC_TEXTSIZE
                                if (!tools_strtomac(mac, textmac)) { // Échec de conversion
                                    textmac[MAC_TEXTSIZE] = '\0'; // Remplacement par '\0' (l'octet existe)
                                    log(KERN_ERR, "Invalid MAC address %s", textmac);
                                    goto NEXTLINE;
                                }
                                line += MAC_TEXTSIZE + 1;
                                linesize -= MAC_TEXTSIZE + 1;
                                textip = line; // Adresse IPvX sur linesize
                                if (version == 4) { // IPv4
                                    if (!tools_strtoipv4(ip, textip)) { // Échec de conversion
                                        textmac[MAC_TEXTSIZE] = '\0'; // Remplacement par '\0' (l'octet existe)
                                        log(KERN_ERR, "Invalid IPv4 address for %s", textmac);
                                        goto NEXTLINE;
                                    }
                                } else { // IPv6
                                    if (!tools_strtoipv6((nint16*) ip, textip)) { // Échec de conversion
                                        textmac[MAC_TEXTSIZE] = '\0'; // Remplacement par '\0' (l'octet existe)
                                        log(KERN_ERR, "Invalid IPv6 address for %s", textmac);
                                        goto NEXTLINE;
                                    }
                                }
                            }
                            if (add) { // Ajout
                                struct tuple* tuple = scheduler_gettuple(mac, ip, version); // Récupération du tuple référencé
                                if (tuple) { // Existe déjà
                                    tuple_unref(tuple); /// UNREF
                                    goto NEXTLINE;
                                }
                                tuple = tuple_create(GFP_KERNEL); // Allocation d'un nouveau tuple
                                if (!tuple) { // Échec d'allocation
                                    log(KERN_WARNING, "Tuple allocation failure");
                                    return; // Lignes suivantes également concernées
                                }
                                { // Initialisation du tuple
                                    memcpy(tuple->address.mac, mac, MAC_SIZE); // Copie de la MAC
                                    if (version == 4) {
                                        memcpy(tuple->address.ipv4, ip, IPV4_SIZE); // Copie de l'IPv4
                                    } else {
                                        memcpy(tuple->address.ipv6, ip, IPV6_SIZE); // Copie de l'IPv6
                                    }
                                    tuple->address.version = version;
                                    tuple->member = member; // Spécification de l'adhérent, référencé ci-dessous
                                    member_ref(member); /// REF
                                    if (unlikely(!member_lock(member))) { /// LOCK
                                        tuple_unref(tuple); /// UNREF
                                        return; // Échec aussi pour les autres entrées
                                    }
                                    tuple_ref(tuple); /// REF
                                    list_add_tail(&(tuple->list), &(member->tuples)); // Ajout en fin de liste
                                    member_unlock(member); /// UNLOCK
                                }
                                if (unlikely(!scheduler_inserttuple(tuple))) { // Insertion du tuple
                                    tuple_clean(tuple); // Nettoyage du tuple
                                    tuple_unref(tuple); /// UNREF
                                    return; // Échec aussi pour les autres entrées
                                }
                                tuple_unref(tuple); /// UNREF
                            } else { // Suppression
                                struct tuple* tuple = scheduler_gettuple(mac, ip, version); // Récupération du tuple
                                if (!tuple) // N'existe pas
                                    goto NEXTLINE;
                                tuple_clean(tuple); // Nettoyage du tuple
                                tuple_unref(tuple); /// UNREF
                            }
                        }
                    }
                NEXTLINE:;
                }
            } return;
            case ID_MEMBER_ROUTER: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                if (size <= 1) { // Suppression du routeur
                    member_setrouter(member, null);
                } else { // Application du routeur
                    struct router* router; // Structure du routeur
                    nint id; // Identifiant du routeur
                    if (!tools_strtonint((nint8*) data, size, &id)) // Conversion base 10
                        return;
                    { // Récupération de la structure du routeur
                        struct object_router* obj_gw = control_getbyid_router(id);
                        if (obj_gw) { // Existe
                            router = &(obj_gw->structure);
                        } else {
                            log(KERN_ERR, "Router " NINT " doesn't exist", id);
                            return;
                        }
                    }
                    member_setrouter(member, router);
                }
            } return;
#if FAIRCONF_SCHEDULER_HANDLEMAXLATENCY == 1
            case ID_MEMBER_MAXLATENCY: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                nint maxlatency; // Latence maximale
                if (!tools_strtonint((nint8*) data, size, &maxlatency)) // Conversion base 10
                    return;
                if (unlikely(!member_lock(member))) /// LOCK
                    return;
                member->maxlatency = maxlatency;
                member_unlock(member); /// UNLOCK
            } return;
#endif
            case ID_MEMBER_PRIORITY: {
                struct member* member = control_getmemberbykobject(kobject); // Structure de l'adhérent
                nint priority;
                if (!tools_strtonint((nint8*) data, size, &priority)) // Conversion base 10
                    return;
                if (unlikely(priority == 0)) // Temps réel demandé
                    log(KERN_WARNING, "Real-time priority set to member " NINT, control_getobjectmemberbykobject(kobject)->id);
                if (unlikely(!member_lock(member))) /// LOCK
                    return;
                member->priority = priority;
                member_unlock(member); /// UNLOCK
            } return;
            default: // Attribut inconnu
                log(KERN_ERR, CONTROL_INVALIDSTORE, id);
                return;
        }
    }
    control_lock(); /// LOCK
    object_member_do_store(control_getidbyattr(attr));
    control_unlock(); /// UNLOCK
    return size;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Objet adhérent ▔
/// ▁ Objet scheduler ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Prototype
static void object_scheduler_release(struct kobject*);
static ssize_t object_scheduler_show(struct kobject*, struct attribute*, char*);
static ssize_t object_scheduler_store(struct kobject*, struct attribute*, const char*, size_t);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Liste des attributs
static struct control_attribute object_scheduler_attribute[] = {
#if FAIRCONF_CONNLOG == 1
    { // CONNECTIONS
        .id = ID_SCHED_CONNECTIONS,
        .attribute = {
            .name = LABEL_SCHED_CONNECTIONS,
            .mode = 0444
        }
    },
    { // MAXLOGENTRIES
        .id = ID_SCHED_MAXLOGENTRIES,
        .attribute = {
            .name = LABEL_SCHED_MAXLOGENTRIES,
            .mode = 0666
        }
    },
#endif
    { // MAXCONNPERUSER
        .id = ID_SCHED_MAXCONNPERUSER,
        .attribute = {
            .name = LABEL_SCHED_MAXCONNPERUSER,
            .mode = 0666
        }
    },
    { // CREATEADHERENT
        .id = ID_SCHED_CREATEADHERENT,
        .attribute = {
            .name = LABEL_SCHED_CREATEADHERENT,
            .mode = 0222
        }
    },
    { // DELETEADHERENT
        .id = ID_SCHED_DELETEADHERENT,
        .attribute = {
            .name = LABEL_SCHED_DELETEADHERENT,
            .mode = 0222
        }
    },
    { // CREATEROUTER
        .id = ID_SCHED_CREATEROUTER,
        .attribute = {
            .name = LABEL_SCHED_CREATEROUTER,
            .mode = 0222
        }
    },
    { // DELETEROUTER
        .id = ID_SCHED_DELETEROUTER,
        .attribute = {
            .name = LABEL_SCHED_DELETEROUTER,
            .mode = 0222
        }
    }
};

/// Table des attributs
static struct attribute* object_scheduler_attributes[] = {
    &(object_scheduler_attribute[0].attribute),
    &(object_scheduler_attribute[1].attribute),
    &(object_scheduler_attribute[2].attribute),
    &(object_scheduler_attribute[3].attribute),
    &(object_scheduler_attribute[4].attribute),
#if FAIRCONF_CONNLOG == 1
    &(object_scheduler_attribute[5].attribute),
    &(object_scheduler_attribute[6].attribute),
#endif
    null // Toujours terminée par null
};

/// Définition des opérations
static struct sysfs_ops object_scheduler_ops = {
    .show = object_scheduler_show,
    .store = object_scheduler_store
};

/// Définition du type
static struct kobj_type object_scheduler_type = {
    .release = object_scheduler_release,
    .sysfs_ops = &object_scheduler_ops,
    .default_attrs = object_scheduler_attributes,
    .child_ns_type = null,
    .namespace = null
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur création du scheduler.
 * @return Précise si l'opération est un succès
**/
static bool object_scheduler_create(void) {
    if (unlikely(!scheduler_init())) // Échec d'initialisation de la structure
        goto ERR_1;
    if (unlikely(kobject_init_and_add(&(control.kobject), &object_scheduler_type, kernel_kobj, LABEL_SCHED))) // Échec d'initialisation ou ajout
        goto ERR_2;
    if (unlikely(!(control.members = kobject_create_and_add(LABEL_MEMBER, &(control.kobject))))) // Échec d'initialisation ou ajout
        goto ERR_3;
    if (unlikely(!(control.routers = kobject_create_and_add(LABEL_ROUTER, &(control.kobject))))) // Échec d'initialisation ou ajout
        goto ERR_4;
#if FAIRCONF_CONNLOG == 1
    if (unlikely(!connlog_init(&(control.connlog)))) // Échec d'initialisation des logs de connexions
        goto ERR_5;
#endif
    INIT_LIST_HEAD(&(control.list_object_member)); // Initialisation...
    INIT_LIST_HEAD(&(control.list_object_router)); // ...des listes
    mutex_init(&(control.lock)); // Initialisation de l'objet de synchronisation
    return true;
#if FAIRCONF_CONNLOG == 1
    ERR_5: kobject_put(control.routers); // Libération du kobject
#endif
    ERR_4: kobject_put(control.members); // Libération du kobject
    ERR_3: kobject_put(&(control.kobject)); // Libération du kobject
    ERR_2: scheduler_clean(); // Nettoyage de la structure
    ERR_1: return false;
}

/** Sur libération du scheduler, destructeur du kobject.
 * @param kobject Pointeur sur l'objet scheduler
**/
static void object_scheduler_release(struct kobject* kobject) {
#if FAIRCONF_CONNLOG == 1
    connlog_clean(&(control.connlog)); // Nettoyage des logs de connexions
#endif
    scheduler_clean(); // Nettoyage de la structure
    scheduler_unref(&scheduler); /// UNREF
}

/** Sur lecture des données.
 * @param kobject Pointeur sur le kobject concerné
 * @param attr    Structure d'attribut
 * @param data    Buffer de données reçues
 * @param size    Taille du buffer
 * @return Nombre d'octets lus
**/
static ssize_t object_scheduler_show(struct kobject* kobject, struct attribute* attr, char* data) {
    ssize_t object_scheduler_do_show(nint id) {
        switch (id) {
#if FAIRCONF_CONNLOG == 1
            case ID_SCHED_CONNECTIONS: {
                struct object_scheduler* scheduler = control_getobjectschedulerbykobject(kobject); // Objet scheduler
                char* cursor = data; // Curseur
                nint totalsize = 0, size; // Taille totale, taille du buffer
                struct connlog* connlog = &(scheduler->connlog); // Structure des logs
                while (true) {
                    size = connlog_cache(connlog); // Mise en cache de la chaîne
                    if (size == 0) { // Fin de construction
                        break;
                    } else if (totalsize + size <= PAGE_SIZE) { // Rentre dans la page
                        nint i;
                        for (i = 0; i < size; i++) // Copie du buffer
                            cursor[i] = connlog->buffer[i];
                        totalsize += size; // Ajout de la taille
                        cursor += size; // Déplacement du curseur
                        connlog_pop(connlog); // Suppression de l'entrée
                    } else { // Page pleine
                        break;
                    }
                }
                return totalsize;
            }
            case ID_SCHED_MAXLOGENTRIES: {
                return sprintf(data, NINT "\n", connlog_getlimit(&(control_getobjectschedulerbykobject(kobject)->connlog)));
            }
#endif
            case ID_SCHED_MAXCONNPERUSER: {
                nint maxconn; // Nombre maximal de connexions
                if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
                    return 0;
                maxconn = scheduler.maxconnections;
                scheduler_unlock(&scheduler); /// UNLOCK
                return sprintf(data, NINT "\n", maxconn);
            }
            default: // Attribut inconnu
                log(KERN_ERR, CONTROL_INVALIDSHOW, id);
                return 0;
        }
    }
    ssize_t size; // Taille écrite
    control_lock(); /// LOCK
    size = object_scheduler_do_show(control_getidbyattr(attr));
    control_unlock(); /// UNLOCK
    return size;
}

/** Sur écriture de données.
 * @param kobject Pointeur sur le kobject concerné
 * @param attr    Structure d'attribut
 * @param data    Buffer de données reçues
 * @param size    Taille du buffer
 * @return Nombre d'octets écrits
**/
static ssize_t object_scheduler_store(struct kobject* kobject, struct attribute* attr, const char* data, size_t size) {
    void object_scheduler_do_store(nint id) {
        switch (id) {
            case ID_SCHED_MAXCONNPERUSER: {
                nint value;
                if (!tools_strtonint((nint8*) data, size, &value)) // Conversion base 10
                    return;
                if (value == 0) { // Aucune connexion demandée
                    log(KERN_ERR, "At least one connection per user must be allowed");
                    return;
                }
                if (unlikely(!scheduler_lock(&scheduler))) /// LOCK
                    return;
                scheduler.maxconnections = value; // Inscription de la nouvelle valeur
                scheduler_unlock(&scheduler); /// UNLOCK
            } return;
#if FAIRCONF_CONNLOG == 1
            case ID_SCHED_MAXLOGENTRIES: {
                nint value;
                if (!tools_strtonint((nint8*) data, size, &value)) // Conversion base 10
                    return;
                connlog_setlimit(&(control_getobjectschedulerbykobject(kobject)->connlog), value); // Affectation de la valeur
            } return;
#endif
            case ID_SCHED_CREATEADHERENT: {
                nint id;
                if (!tools_strtonint((nint8*) data, size, &id)) // Conversion base 10
                    return;
                if (!object_member_create(id)) // Échec de création
                    log(KERN_ERR, "Creation of member " NINT " failed", id);
            } return;
            case ID_SCHED_DELETEADHERENT: {
                struct object_member* object_member; // Objet adhérent
                nint id;
                if (!tools_strtonint((nint8*) data, size, &id)) // Conversion base 10
                    return;
                object_member = control_getbyid_member(id); // Obtention de l'adhérent
                if (!object_member) { // Non trouvé
                    log(KERN_ERR, "Adherent " NINT " not found", id);
                    return;
                }
                kobject_put(&(object_member->kobject)); // Libération de l'objet et nettoyage de la structure
            } return;
            case ID_SCHED_CREATEROUTER: {
                nint id;
                if (!tools_strtonint((nint8*) data, size, &id)) // Conversion base 10
                    return;
                if (!object_router_create(id)) // Échec de création
                    log(KERN_ERR, "Creation of router " NINT " failed", id);
            } return;
            case ID_SCHED_DELETEROUTER: {
                struct object_router* object_router; // Objet routeur
                nint id;
                if (!tools_strtonint((nint8*) data, size, &id)) // Conversion base 10
                    return;
                object_router = control_getbyid_router(id); // Obtention du routeur
                if (!object_router) { // Non trouvé
                    log(KERN_ERR, "Router " NINT " not found", id);
                    return;
                }
                kobject_put(&(object_router->kobject)); // Libération de l'objet et nettoyage de la structure
            } return;
            default: // Attribut inconnu
                log(KERN_ERR, CONTROL_INVALIDSTORE, id);
                return;
        }
    }
    control_lock(); /// LOCK
    object_scheduler_do_store(control_getidbyattr(attr));
    control_unlock(); /// UNLOCK
    return size; // Toujours retourner
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Objet scheduler ▔
/// ▁ Contrôle ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Variables globales
struct object_scheduler control;   // Structure de contrôle
struct scheduler        scheduler; // Structure du scheduler

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Obtient un routeur par son identifiant.
 * @param id Identifiant du routeur
 * @return Pointeur sur l'objet routeur (null si inexistant)
**/
static struct object_router* control_getbyid_router(nint id) {
    struct object_router* router; // Structure adhérent en cours
    list_for_each_entry(router, &(control.list_object_router), list) {
        if (router->id == id) // Adhérent trouvé
            return router;
    }
    return null; // Adhérent non trouvé
}

/** Obtient un adhérent par son identifiant.
 * @param id Identifiant de l'adhérent
 * @return Pointeur sur l'objet adhérent (null si inexistant)
**/
static struct object_member* control_getbyid_member(nint id) {
    struct object_member* member; // Structure adhérent en cours
    list_for_each_entry(member, &(control.list_object_member), list) {
        if (member->id == id) // Adhérent trouvé
            return member;
    }
    return null; // Adhérent non trouvé
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Crée l'arborescence de contrôle.
 * @return Structure du scheduler, null si échec
**/
bool control_create(void) {
    return object_scheduler_create(); // Simple wrapper
}

/** Détruit l'arborescence de contrôle.
**/
void control_destroy(void) {
    control_lock(); /// LOCK
    { // Destruction des adhérents
        struct object_member* member; // Adhérent en cours
        struct object_member* next; // Adhérent suivant (seulement pour le safe)
        list_for_each_entry_safe(member, next, &(control.list_object_member), list) // Passage sur tous les adhérents
            kobject_put(&(member->kobject)); // Libération de l'objet et nettoyage de la structure
    }
    { // Destruction des routeurs
        struct object_router* router; // Routeur en cours
        struct object_router* next; // Routeur suivante (seulement pour le safe)
        list_for_each_entry_safe(router, next, &(control.list_object_router), list) // Passage sur toutes les routeurs
            kobject_put(&(router->kobject)); // Libération de l'objet et nettoyage de la structure
    }
    kobject_put(control.members); // Libération...
    kobject_put(control.routers); // ...des répertoires
    kobject_put(&(control.kobject)); // Libération de l'objet scheduler
    control_unlock(); /// UNLOCK
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Contrôle ▔
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
