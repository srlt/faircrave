#ifndef SRC_CONTROL_H
#define SRC_CONTROL_H
/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▁ Documentation ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/**
 * @file   control.h
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
 * Gère la communication avec le programme de contrôle (via le sysfs), et effectue les modifications.
**/

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Documentation ▔
/// ▁ Déclarations ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Headers externes
#include <linux/mutex.h>
#include <linux/kobject.h>

/// Headers internes
#include "types.h"
#include "config.h"
#include "tools.h"
#include "scheduler.h"

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Variables globales
extern struct object_scheduler control;   // Structure de contrôle
extern struct scheduler        scheduler; // Structure du scheduler

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Déclarations ▔
/// ▁ Log des connexions ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

#if FAIRCONF_CONNLOG == 1

/// Métriques
#define CONNLOG_BUFFERSIZE 256 // Taille du buffer (en octets)
#define CONNLOG_LOGENTRIES FAIRCONF_CONNLOG_LOGENTRIES // Limite initial du nombre d'entrées

/// Protocoles
#define CONNLOG_PROTO_TCPIPv4 0 // TCP/IPv4
#define CONNLOG_PROTO_UDPIPv4 1 // UDP/IPv4
#define CONNLOG_PROTO_TCPIPv6 2 // TCP/IPv6
#define CONNLOG_PROTO_UDPIPv6 3 // UDP/IPv6

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Entrée de log d'ouverture/fermeture d'une connexion
struct connlog_entry {
    struct list_head list; // Liste des entrées
    nint member;   // Identifiant de l'adhérent
    nint router;   // Identifiant du routeur
    nint date;     // Date de l'entrée (epoch)
    nint duration; // Durée de la connexion (en secondes)
    struct {
        nint16 protocol; // Protocole utilisé
        nint16 port;     // Port cible
        union {
            nint8 ipv4[IPV4_SIZE]; // Version 4
            nint8 ipv6[IPV6_SIZE]; // Version 6
        } address; // Adresse IP de la cible
    } target; // Information sur la cible
};

/// Structure de log d'ouverture/fermeture d'une connexion
struct connlog {
    struct spinlock  lock; // Verrou d'accès
    struct list_head list; // Liste des entrées
    aint   count;  // Nombre d'entrées
    aint   limit;  // Limite du nombre d'entrée
    nint   size;   // Taille utilisée du buffer
    nint8* buffer; // Buffer de la première connexion
    struct kmem_cache* slab; // Slab des logs de connexion
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

bool connlog_push(struct connlog*, nint, nint, nint, nint16, nint8*, nint16);

#endif

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Log des connexions ▔
/// ▁ Objet routeur ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Identifiants
#define ID_ROUTER_STATUS    0 // Statut du routeur, ordre de fermeture
#define ID_ROUTER_NETDEVICE 1 // Statut du routeur, ordre de fermeture
#define ID_ROUTER_LATENCY   2 // Latence mesurée
#define ID_ROUTER_ALLOWIPV4 3 // IPv4 pris en charge
#define ID_ROUTER_ALLOWIPV6 4 // IPv6 pris en charge
#define ID_ROUTER_TPUTLIMIT 5 // Limite de débit montant, en ko/s
#define ID_ROUTER_TPUTUP    6 // Débit montant
#define ID_ROUTER_TPUTDOWN  7 // Débit descendant

/// Labels associés
#define LABEL_ROUTER "routers"
#define LABEL_ROUTER_STATUS    "status"
#define LABEL_ROUTER_NETDEVICE "netdevice"
#define LABEL_ROUTER_LATENCY   "latency"
#define LABEL_ROUTER_ALLOWIPV4 "allow_ipv4"
#define LABEL_ROUTER_ALLOWIPV6 "allow_ipv6"
#define LABEL_ROUTER_TPUTLIMIT "tput_limit"
#define LABEL_ROUTER_TPUTUP    "tput_up"
#define LABEL_ROUTER_TPUTDOWN  "tput_down"

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Objet routeur
struct object_router {
    struct router   structure; // Structure associée
    struct kobject   kobject;  // kobject associé
    struct list_head list;     // Liste des routeurs
    nint id; // Identifiant du routeur
};

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Objet routeur ▔
/// ▁ Objet adhérent ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Identifiants
#define ID_MEMBER_TUPLES     0 // Lecture/ajout/suppression de tuples MAC/IPv4/IPv6
#define ID_MEMBER_ROUTER     1 // Routeur fixée
#define ID_MEMBER_LATENCY    2 // Latence actuelle (interne au scheduler)
#define ID_MEMBER_MAXLATENCY 3 // Latences maximales admissibles (avec UDP seulement)
#define ID_MEMBER_PRIORITY   4 // Priorité de l'utilisateur
#define ID_MEMBER_TPUTASK    5 // Débit montant demandée par l'adhérent
#define ID_MEMBER_TPUTLOST   6 // Débit montant perdu
#define ID_MEMBER_TPUTUP     7 // Débit montant obtenu
#define ID_MEMBER_TPUTDOWN   8 // Débit descendant de l'adhérent

/// Labels associés
#define LABEL_MEMBER "members"
#define LABEL_MEMBER_TUPLES     "tuples"
#define LABEL_MEMBER_ROUTER     "router"
#define LABEL_MEMBER_LATENCY    "latency"
#define LABEL_MEMBER_MAXLATENCY "maxlatency"
#define LABEL_MEMBER_PRIORITY   "priority"
#define LABEL_MEMBER_TPUTASK    "tput_ask"
#define LABEL_MEMBER_TPUTLOST   "tput_lost"
#define LABEL_MEMBER_TPUTUP     "tput_up"
#define LABEL_MEMBER_TPUTDOWN   "tput_down"

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Objet adhérent
struct object_member {
    struct member    structure; // Structure associée
    struct kobject   kobject;   // kobject associé
    struct list_head list;      // Liste des adhérents
    nint id; // Identifiant de l'adhérent
};

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Objet adhérent ▔
/// ▁ Objet scheduler ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Identifiants
#define ID_SCHED_CONNECTIONS    0 // Évènements (données d'ouvertures/fermetures de (pseudo-)connexions, quel adhérent avec quelle IP distante)
#define ID_SCHED_MAXCONNPERUSER 1 // Nombre maximal de connexion par utilisateur
#define ID_SCHED_MAXLOGENTRIES  2 // Nombre maximal d'entrées dans les logs de connexion
#define ID_SCHED_DEFAULTMEMBER  3 // Membre si aucun tuple ne match
#define ID_SCHED_CREATEADHERENT 4 // Création d'un adhérent
#define ID_SCHED_DELETEADHERENT 5 // Suppression d'un adhérent
#define ID_SCHED_CREATEROUTER   6 // Création d'un routeur
#define ID_SCHED_DELETEROUTER   7 // Suppression d'un routeur

/// Labels associés
#define LABEL_SCHED "faircrave"
#define LABEL_SCHED_CONNECTIONS    "connections"
#define LABEL_SCHED_MAXCONNPERUSER "maxconnperuser"
#define LABEL_SCHED_MAXLOGENTRIES  "maxlogentries"
#define LABEL_SCHED_DEFAULTMEMBER  "default_member"
#define LABEL_SCHED_CREATEADHERENT "create_member"
#define LABEL_SCHED_DELETEADHERENT "delete_member"
#define LABEL_SCHED_CREATEROUTER   "create_router"
#define LABEL_SCHED_DELETEROUTER   "delete_router"

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Objet scheduler (la structure est séparée)
struct object_scheduler {
    struct mutex     lock;                // Mutex d'accès au contrôle
    struct list_head list_object_member;  // Liste des objets adhérents
    struct list_head list_object_router; // Liste des objets routeurs
    struct kobject   kobject;             // kobject associé
    struct kobject*  members;             // Répertoire des adhérents
    struct kobject*  routers;             // Répertoire des routeurs
    struct connlog   connlog;             // Logs des connexions
};

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Objet scheduler ▔
/// ▁ Contrôle ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Structure d'attribut
struct control_attribute {
    nint id; // Identifiant
    struct attribute attribute; // Structure d'attribut
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

bool control_create(void);
void control_destroy(void);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Obtient l'objet par la structure.
 * @param <structure> Pointeur sur la structure
 * @return Objet
**/
static inline struct object_router* control_getobjectrouterbystructure(struct router* router) {
    return container_of(router, struct object_router, structure);
}
static inline struct object_member* control_getobjectmemberbystructure(struct member* member) {
    return container_of(member, struct object_member, structure);
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

#if FAIRCONF_CONNLOG == 1

/** Log une connexion, à appeler lors de sa fin.
 * @param member   Identifiant de l'adhérent
 * @param router   Identifiant du routeur
 * @param duration Durée de la connexion (en secondes)
 * @param protocol Code du protocole
 * @param address  Adresse IPv4/v6 cible
 * @param port     Port cible
 * @return Précise si l'opération est un succès
**/
static inline bool control_logconnection(nint member, nint router, nint duration, nint16 protocol, nint8* address, nint16 port) {
    return connlog_push(&(control.connlog), member, router, duration, protocol, address, port);
}

#endif

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Contrôle ▔
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
#endif
