#ifndef SRC_TOOLS_H
#define SRC_TOOLS_H
/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▁ Documentation ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/**
 * @file   tools.h
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
**/

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Documentation ▔
/// ▁ Déclarations ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Headers externes
#include <linux/slab.h>
#include <linux/atomic.h>

/// Headers internes
#include "types.h"
#include "config.h"

/// Divers
#undef current // Nom utilisé dans le code (utiliser get_current())

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Déclarations ▔
/// ▁ Divers ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// FIXME: Résolution du timer suffisant pour un routeur QoS ? Voir http://lxr.free-electrons.com/source/include/net/pkt_sched.h?v=3.2#L23
/// NOTE: Possibilité d'utiliser un timer 1 µs précision. L'exactitude du débit en sortie n'est cependant pas capitale dans notre cas.

/// Logs
#define log(level, text, ...) printk(level "[FairCrave] " text " --- %s (%s:%u)\n", ## __VA_ARGS__, __FUNCTION__, __FILENAME__, __LINE__) // Log d'une chaîne préformattée

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Renvoie la date (en jiffies).
 * @return Date lors de l'appel (en jiffies)
**/
static inline nint time_now(void) {
#if __LP64__
    return (nint) get_jiffies_64(); // De unsigned long à nint, toujours
#else
    return (nint) jiffies;
#endif
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Divers ▔
/// ▁ Verrou d'accès ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Mot avec accès atomiques
#if __LP64__

typedef atomic64_t aint;

#define AINT_INIT(i)             ATOMIC64_INIT(i)
#define aint_read(v)             atomic64_read(v)
#define aint_set(v, i)           atomic64_set(v, i)
#define aint_add(i, v)           atomic64_add(i, v)
#define aint_sub(i, v)           atomic64_sub(i, v)
#define aint_inc(v)              atomic64_inc(v)
#define aint_dec(v)              atomic64_dec(v)
#define aint_add_negative(i, v)  atomic64_add_negative(i, v)
#define aint_add_return(i, v)    atomic64_add_return(i, v)
#define aint_sub_return(i, v)    atomic64_sub_return(i, v)
#define aint_inc_return(v)       atomic64_inc_return(v)
#define aint_dec_return(v)       atomic64_dec_return(v)
#define aint_sub_and_test(i, v)  atomic64_sub_and_test(i, v)
#define aint_dec_and_test(v)     atomic64_dec_and_test(v)
#define aint_inc_and_test(v)     atomic64_inc_and_test(v)
#define aint_cmpxchg(v, o, n)    atomic64_cmpxchg(v, o, n)
#define aint_xchg(v, n)          atomic64_xchg(v, n)
#define aint_add_unless(v, a, u) atomic64_add_unless(v, a, u)
#define aint_inc_not_zero(v)     atomic64_inc_not_zero(v)
#define aint_dec_if_positive(v)  atomic64_dec_if_positive(v)

#else // 32 bits

typedef atomic_t aint;

#define AINT_INIT(i)             ATOMIC_INIT(i)
#define aint_read(v)             atomic_read(v)
#define aint_set(v, i)           atomic_set(v, i)
#define aint_add(i, v)           atomic_add(i, v)
#define aint_sub(i, v)           atomic_sub(i, v)
#define aint_inc(v)              atomic_inc(v)
#define aint_dec(v)              atomic_dec(v)
#define aint_add_negative(i, v)  atomic_add_negative(i, v)
#define aint_add_return(i, v)    atomic_add_return(i, v)
#define aint_sub_return(i, v)    atomic_sub_return(i, v)
#define aint_inc_return(v)       atomic_inc_return(v)
#define aint_dec_return(v)       atomic_dec_return(v)
#define aint_sub_and_test(i, v)  atomic_sub_and_test(i, v)
#define aint_dec_and_test(v)     atomic_dec_and_test(v)
#define aint_inc_and_test(v)     atomic_inc_and_test(v)
#define aint_cmpxchg(v, o, n)    atomic_cmpxchg(v, o, n)
#define aint_xchg(v, n)          atomic_xchg(v, n)
#define aint_add_unless(v, a, u) atomic_add_unless(v, a, u)
#define aint_inc_not_zero(v)     atomic_inc_not_zero(v)
#define aint_dec_if_positive(v)  atomic_dec_if_positive(v)

#endif

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Statuts de l'objet
#define ACCESS_STATUS_CLOSE  0 // Objet fermé, ne pouvant être verrouillé
#define ACCESS_STATUS_OPEN   1 // Objet ouvert, pouvant être verrouillé

/** Initialise un verrou d'accès statiquement.
 * @param name  Nom de la variable (pour depmap si utilisé)
**/
#define ACCESS_INIT(name) \
    { \
        .status  = ATOMIC_INIT(ACCESS_STATUS_OPEN), \
        .refs    = ATOMIC_INIT(1), \
        .wait    = ATOMIC_INIT(0), \
        .param   = 0, \
        .destroy = null, \
        .lock    = __SPIN_LOCK_UNLOCKED(name) \
    }

/** Définie les fonctions d'accès pour un objet.
 * @param type   Nom du type de la structure
 * @param member Nom du membre access de la structure
**/
#define ACCESS_DEFINE(type, member) \
    static inline void type##_open(struct type* object, void (*destroy)(struct access*, zint), zint param) { \
        ACCESS_TRACE_OPEN(object); \
        access_open(&(object->member), destroy, param); \
    } \
    static inline void type##_close(struct type* object) { \
        ACCESS_TRACE_CLOSE(object); \
        access_close(&(object->member)); \
    } \
    static inline bool type##_isvalid(struct type* object) { \
        return access_isvalid(&(object->member)); \
    } \
    static inline void type##_ref(struct type* object) { \
        ACCESS_TRACE_REF(object); \
        access_ref(&(object->member)); \
    } \
    static inline void type##_unref(struct type* object) { \
        ACCESS_TRACE_UNREF(object); \
        access_unref(&(object->member)); \
    } \
    static inline bool type##_lock(struct type* object) { \
        ACCESS_TRACE_LOCK(object); \
        return access_lock(&(object->member)); \
    } \
    static inline void type##_unlock(struct type* object) { \
        ACCESS_TRACE_UNLOCK(object); \
        access_unlock(&(object->member)); \
    }

#if FAIRCONF_ACCESS_TRACEOPEN == 1
    #define ACCESS_TRACE_OPEN(object)  log(KERN_DEBUG, "OPEN   %p", object)
    #define ACCESS_TRACE_CLOSE(object) log(KERN_DEBUG, "CLOSE  %p", object)
#else
    #define ACCESS_TRACE_OPEN(object)  do {} while(0)
    #define ACCESS_TRACE_CLOSE(object) do {} while(0)
#endif

#if FAIRCONF_ACCESS_TRACEREF == 1
    #define ACCESS_TRACE_REF(object)   log(KERN_DEBUG, "REF    %p", object)
    #define ACCESS_TRACE_UNREF(object) log(KERN_DEBUG, "UNREF  %p", object)
#else
    #define ACCESS_TRACE_REF(object)   do {} while(0)
    #define ACCESS_TRACE_UNREF(object) do {} while(0)
#endif

#if FAIRCONF_ACCESS_TRACELOCK == 1
    #define ACCESS_TRACE_LOCK(object)   log(KERN_DEBUG, "LOCK   %p", object)
    #define ACCESS_TRACE_UNLOCK(object) log(KERN_DEBUG, "UNLOCK %p", object)
#else
    #define ACCESS_TRACE_LOCK(object)   do {} while(0)
    #define ACCESS_TRACE_UNLOCK(object) do {} while(0)
#endif

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Verrou d'accès (/!\ unique par structure)
struct access {
    aint status; // Status du verrou d'accès
    aint refs;   // Nombre de références
    aint wait;   // Nombre d'attente de verrouillage
    zint param;  // Paramètre du destructeur
    void (*destroy)(struct access*, zint); // Destructeur
    struct spinlock lock; // Verrou d'accès
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

extern aint access_warnrefcount;
extern aint access_warnopencount;

void access_warninit(void);
void access_warnclean(void);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Ouvre un verrou d'accès dynamiquement, référencé, ne peut-être fait qu'une fois.
 * @param access  Pointeur sur le verrou d'accès, en cours d'initialisation
 * @param destroy Fonction de destruction associée (peut-être null)
 * @param param   Paramètre du destructeur
**/
static inline void access_open(struct access* access, void (*destroy)(struct access*, zint), zint param) {
#if FAIRCONF_ACCESS_WARNREF == 1
    aint_inc(&access_warnrefcount); // Compte
#endif
#if FAIRCONF_ACCESS_WARNOPEN == 1
    aint_inc(&access_warnopencount); // Compte
#endif
    aint_set(&(access->status), ACCESS_STATUS_OPEN);
    aint_set(&(access->refs), 1);
    aint_set(&(access->wait), 0);
    spin_lock_init(&(access->lock));
    access->destroy = destroy;
    access->param = param;
}

/** Ferme un verrou d'accès, fait barrière aux tentatives d'acquisitions, ne déverrouille pas l'objet.
 * @param access Pointeur sur le verrou d'accès, référencé et acquis par l'appelant
**/
static inline void access_close(struct access* access) {
#if FAIRCONF_ACCESS_WARNOPEN == 1
    aint_dec(&access_warnopencount); // Décompte
    if (unlikely(aint_read(&(access->status)) == ACCESS_STATUS_CLOSE)) // Objet déjà fermé
        log(KERN_CRIT, "Closing already-closed object %p", access);
#endif
    aint_set(&(access->status), ACCESS_STATUS_CLOSE); // Fermeture
    while (aint_read(&(access->wait)) != 0) // Busy-wait des tentatives d'acquisition
        cpu_relax();
}

/** Destructeur par kfree (/!\ uniquement si access en tête de structure).
 * @param access Structure du verrou d'accès, en destruction
 * @param param  Paramètre du destructeur (ignoré)
**/
static void as(unused) access_kfree(struct access* access, zint param) {
    kfree(access); // Libération
}

/** Précise si le verrou peut-être utilisé, donc s'il est ouvert.
 * @param access Structure du verrou d'accès, référencé par l'appelant ou en cours d'initialisation
 * @return Vrai si l'objet est ouvert, faux sinon
**/
static inline bool access_isvalid(struct access* access) {
    return aint_read(&(access->status)) == ACCESS_STATUS_OPEN;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Référence le verrou d'accès, permet le maintien de l'objet en mémoire.
 * @param access Structure du verrou d'accès, référencé par l'appelant ou en cours d'initialisation
**/
static inline void access_ref(struct access* access) {
#if FAIRCONF_ACCESS_WARNREF == 1
    aint_inc(&access_warnrefcount); // Compte
    if (unlikely(aint_inc_not_zero(&(access->refs)) == 0)) // Référencement d'un objet non déjà référencé
        log(KERN_CRIT, "Reference on null-referenced object %p", access);
#else
    aint_inc(&(access->refs)); // Référencement
#endif
}

/** "Déréférence" le verrou d'accès, libère l'objet si nécessaire.
 * @param access Structure du verrou d'accès, référencé par l'appelant
**/
static inline void access_unref(struct access* access) {
#if FAIRCONF_ACCESS_WARNREF == 1
    zint val = aint_dec_return(&(access->refs));
    if (unlikely(val < 0)) // Sur-déréférencement
        log(KERN_CRIT, "Dereferencing null-referenced object %p", access);
    aint_dec(&access_warnrefcount); // Décompte
    if (val == 0) { // Plus aucune référence
#else
    if (aint_dec_return(&(access->refs)) == 0) { // Déréférencement et plus aucune référence
#endif
        if (access->destroy) // Fonction de destruction spécifiée
            access->destroy(access, access->param); // Destruction de l'objet
    }
}

/** Acquisition du verrou d'accès.
 * @param access Structure du verrou d'accès, référencé mais non acquis par l'appelant
 * @return Vrai si l'objet est acquis, faux sinon
**/
static inline bool access_lock(struct access* access) {
    aint_inc(&(access->wait)); // Incrément du compteur d'attente
    while (true) { // Tentative de verrouillage
        if (!access_isvalid(access)) { // Objet devenu fermé
            aint_dec(&(access->wait)); // Décrément du compteur d'attente
            return false;
        }
        if (likely(spin_trylock(&(access->lock)) == 0)) // Verrou acquis
            break;
        cpu_relax();
    }
    aint_dec(&(access->wait)); // Décrément du compteur d'attente
    return true;
}

/** Libération du verrou d'accès.
 * @param access Structure du verrou d'accès, référencé et acquis par l'appelant
**/
static inline void access_unlock(struct access* access) {
    spin_unlock(&(access->lock)); // Déverrouillage
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Verrou d'accès ▔
/// ▁ Conversions ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Protocoles IP
#define MAC_SIZE  6  // Taille d'une adresse MAC (en octets)
#define IPV4_SIZE 4  // Taille d'une adresse IPv4 (en octets)
#define IPV6_SIZE 16 // Taille d'une adresse IPv6 (en octets)
#define TOOLS_PARSE_MAC  0 // Adresse MAC en paramètre
#define TOOLS_PARSE_IPV4 1 // Adresse IPv4 en paramètre
#define TOOLS_PARSE_IPV6 2 // Adresse IPv6 en paramètre

/// Conversion en texte
#define MAC_TEXTSIZE  17 // Taille d'une adresse MAC
#define IPV4_TEXTSIZE 15 // Taille maximale d'une adresse IPv4
#define IPV6_TEXTSIZE 39 // Taille maximale d'une adresse IPv6

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

bool tools_strtonint(nint8*, nint, nint*);

nint tools_mactostr(nint8*, nint8*);
bool tools_strtomac(nint8*, nint8*);
nint tools_ipv4tostr(nint8*, nint8*);
bool tools_strtoipv4(nint8*, nint8*);
nint tools_ipv6tostr(nint8*, nint16*);
bool tools_strtoipv6(nint16*, nint8*);

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Conversions ▔
/// ▁ Liste ordonnée ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Dimensions
#define SORTLIST_TYPE     FAIRCONF_SORTLIST_TYPE     // Type utilisé                              = type
#define SORTLIST_LEVELS   FAIRCONF_SORTLIST_LEVELS   // Nombre de niveaux (doit être non nul)     = levels
#define SORTLIST_SIZE     FAIRCONF_SORTLIST_SIZE     // Nombre de sous-éléments par index/section = 8 * sizeof(type)
#define SORTLIST_SECTIONS FAIRCONF_SORTLIST_SECTIONS // Nombre de sections                        = size^(levels - 1)
#define SORTLIST_INDEX    FAIRCONF_SORTLIST_INDEX    // Nombre d'index                            = (sections - 1) / (size - 1)
#define SORTLIST_LENGTH   FAIRCONF_SORTLIST_LENGTH   // Nombre d'emplacements                     = size * sections

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Header d'index ou de section d'objets.
struct sortlist_header {
    struct sortlist_index* parent; // Index parent (null si racine)
    SORTLIST_TYPE used;   // Masque d'utilisation
    nint          offset; // Offset pour le parent
};

/// Index d'index ou de sections d'objets.
struct sortlist_index {
    struct sortlist_header header; // Header
    union {
        struct sortlist_header*  header;  // Comme header
        struct sortlist_index*   index;   // Comme section
        struct sortlist_section* section; // Comme index
    } children[SORTLIST_SIZE]; // Index/sections/headers enfants
};

/// Section d'objets.
struct sortlist_section {
    struct sortlist_header header;                 // Header
    struct list_head       objects[SORTLIST_SIZE]; // Objets enfant
};

/// Structure de liste ordonnée.
struct sortlist {
    nint phase; // Déphasage du 0 logique par rapport au 0 du tableau
    struct sortlist_section* current;                     // Section en cours
    struct sortlist_index    index[SORTLIST_INDEX];       // Tableau des index
    struct sortlist_section  sections[SORTLIST_SECTIONS]; // Tableau des sections
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

void sortlist_init(struct sortlist*);
bool sortlist_push(struct sortlist*, struct list_head*, nint);
struct list_head* sortlist_get(struct sortlist*);
struct list_head* sortlist_pop(struct sortlist*);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Précise si la liste ordonnée est vide.
 * @param list Liste ordonnée cible
 * @return Vrai si la liste est vide
**/
static inline bool sortlist_empty(struct sortlist* list) {
    return list->index->header.used == 0;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Liste ordonnée ▔
/// ▁ Débit moyen glissant ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Configuration
#define THROUGHPUT_TIME      FAIRCONF_THROUGHPUT_TIME      // Fenêtre de temps de moyennage (en ms)
#define THROUGHPUT_PRECISION FAIRCONF_THROUGHPUT_PRECISION // Nombre de divisions de cette fenêtre
#define THROUGHPUT_WINDOW    (THROUGHPUT_TIME * HZ / 1000)                   // Fenêtre de temps de moyennage (en jiffies)
#define THROUGHPUT_DELTA     (THROUGHPUT_WINDOW / THROUGHPUT_PRECISION)      // Fenêtre de temps d'un bloc (en jiffies)
#define THROUGHPUT_BASE      ((THROUGHPUT_PRECISION - 1) * THROUGHPUT_DELTA) // Fenêtre de temps de "base" de moyennage (en jiffies)

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Structure de débit moyen glissant
struct throughput {
    nint head; // Offset du premier bloc non utilisé
    nint tail; // Offset du dernier bloc utilisé
    zint sum;  // Somme des valeurs (valide après rotation)
    nint64 zero; // Date de référence (en jiffies)
    nint64 time; // Somme du temps écoulé (valide après rotation)
    nint64 last; // Date de la dernière rotation
    zint values[THROUGHPUT_PRECISION]; // Valeurs sommées
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

void throughput_init(struct throughput*);
void throughput_add(struct throughput*, zint);
zint throughput_get(struct throughput*);

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Débit moyen glissant ▔
/// ▁ Moyenne glissante ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Configuration
#define AVERAGE_TIME      THROUGHPUT_TIME      // Fenêtre de temps de moyennage (en ms)
#define AVERAGE_PRECISION THROUGHPUT_PRECISION // Nombre de divisions de cette fenêtre
#define AVERAGE_WINDOW    (AVERAGE_TIME * HZ / 1000)           // Fenêtre de temps de moyennage (en jiffies)
#define AVERAGE_DELTA     (AVERAGE_WINDOW / AVERAGE_PRECISION) // Fenêtre de temps d'un bloc (en jiffies)

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Structure de moyenne glissante
struct average {
    nint head; // Offset du premier bloc non utilisé
    nint tail; // Offset du dernier bloc utilisé
    zint sum;  // Somme des valeurs (valide après rotation)
    nint cnt;  // Nombre de valeurs
    nint64 zero; // Date de référence (en jiffies)
    nint64 last; // Date de la dernière rotation
    zint values[THROUGHPUT_PRECISION]; // Valeurs sommées
    nint counts[THROUGHPUT_PRECISION]; // Nombre de valeurs sommées
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

void average_init(struct average*);
void average_add(struct average*, zint);
zint average_get(struct average*);

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Moyenne glissante ▔
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
#endif
