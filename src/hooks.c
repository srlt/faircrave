/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▁ Documentation ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/**
 * @file   hooks.c
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
 * Interface le module entre la partie scheduling et netfilter.
**/

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Documentation ▔
/// ▁ Déclarations ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Headers externes
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <linux/timer.h>
#include <net/net_namespace.h>
#include <net/pkt_sched.h>
#include <net/netns/ipv4.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <uapi/linux/if_ether.h>

/// Headers internes
#include "hooks.h"
#include "scheduler.h"

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Priorité des hooks
#define HOOKS_PRIORITY ((NF_IP_PRI_CONNTRACK + NF_IP_PRI_MANGLE) / 2) // Après CONNTRACK mais avant MANGLE

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Déclarations ▔
/// ▁ Input pre-mangle post-conntrack ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Prototypes
static unsigned int hooks_input(const struct nf_hook_ops*, struct sk_buff*, const struct net_device*, const struct net_device*, int (*)(struct sk_buff*));

/// Variables
static struct nf_hook_ops hooks_input_ipv4_ops __read_mostly = {
    .hook     = hooks_input,
    .owner    = THIS_MODULE,
    .pf       = NFPROTO_IPV4,
    .hooknum  = NF_INET_PRE_ROUTING,
    .priority = HOOKS_PRIORITY
}; // Opérations de hook pour IPv4
static struct nf_hook_ops hooks_input_ipv6_ops __read_mostly = {
    .hook     = hooks_input,
    .owner    = THIS_MODULE,
    .pf       = NFPROTO_IPV6,
    .hooknum  = NF_INET_PRE_ROUTING,
    .priority = HOOKS_PRIORITY
}; // Opérations de hook pour IPv6

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur input pre-mangle post-conntrack.
 * @param ops  Opération des hooks
 * @param skb  Socket buffer reçu
 * @param in   Network device en entrée
 * @param out  Network device en sortie (null ici)
 * @param okfn Fonction sur acceptation (ignorée)
 * @return Action sur le paquet
**/
static unsigned int hooks_input(const struct nf_hook_ops* ops, struct sk_buff* skb, const struct net_device* in, const struct net_device* out, int (*okfn)(struct sk_buff*)) {
    nint version = (ops->pf == NFPROTO_IPV4 ? 4 : 6); // Version d'IP
    enum ip_conntrack_info ctinfo; // État de la connexion
    struct nf_conn* ct = nf_ct_get(skb, &ctinfo); // Structure de la connexion
    if (unlikely(!ct)) // Paquet non traqué
        return NF_DROP; // = paquet supprimé
    if (unlikely(ctinfo == IP_CT_NEW)) { // Est une nouvelle connexion
        if (!scheduler_interface_input(skb, ct, version)) // Mark refusée
            return NF_DROP;
    }
    skb->mark = ct->mark; // Affectation de la mark
	return NF_ACCEPT;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Input pre-mangle post-conntrack ▔
/// ▁ Forward pre-mangle post-routing ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Prototypes
static unsigned int hooks_forward(const struct nf_hook_ops*, struct sk_buff*, const struct net_device*, const struct net_device*, int (*)(struct sk_buff*));

/// Variables
static struct nf_hook_ops hooks_forward_ipv4_ops __read_mostly = {
    .hook     = hooks_forward,
    .owner    = THIS_MODULE,
    .pf       = NFPROTO_IPV4,
    .hooknum  = NF_INET_FORWARD,
    .priority = HOOKS_PRIORITY
}; // Opérations de hook pour IPv4
static struct nf_hook_ops hooks_forward_ipv6_ops __read_mostly = {
    .hook     = hooks_forward,
    .owner    = THIS_MODULE,
    .pf       = NFPROTO_IPV6,
    .hooknum  = NF_INET_FORWARD,
    .priority = HOOKS_PRIORITY
}; // Opérations de hook pour IPv6

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur forward pre-mangle post-routing.
 * @param ops  Opération des hooks
 * @param skb  Socket buffer reçu
 * @param in   Network device en entrée
 * @param out  Network device en sortie
 * @param okfn Fonction sur acceptation (ignorée)
 * @return Action sur le paquet
**/
static unsigned int hooks_forward(const struct nf_hook_ops* ops, struct sk_buff* skb, const struct net_device* in, const struct net_device* out, int (*okfn)(struct sk_buff*)) {
    nint version = (ops->pf == NFPROTO_IPV4 ? 4 : 6); // Version d'IP
    enum ip_conntrack_info ctinfo; // État de la connexion
    struct nf_conn* ct = nf_ct_get(skb, &ctinfo); // Structure de la connexion (forcément traqué)
    if (unlikely(ctinfo == IP_CT_NEW)) { // Est une nouvelle connexion
        if (!scheduler_interface_forward(skb, ct, version)) // Connexion refusée
            return NF_DROP; // La connexion dans conntrack sera fermée si nécessaire (skbuff.c/skb_release_head_state)
    }
	return NF_ACCEPT;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Forward pre-mangle post-routing ▔
/// ▁ Queuing discipline ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

static int hooks_qdisc_enqueue(struct sk_buff*, struct Qdisc*);
static struct sk_buff* hooks_qdisc_peek(struct Qdisc*);
static struct sk_buff* hooks_qdisc_dequeue(struct Qdisc*);

static int hooks_qdisc_init(struct Qdisc*, struct nlattr*);
static void hooks_qdisc_destroy(struct Qdisc*);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// Paramètres de chaque qdisc
struct hooks_qdiscparam {
    struct netdev* netdev; // Network device liée
    struct {
        nint count; // Nombre de paquets dans la table
        nint pos;   // Position dans la table tournante
        struct sk_buff* table[FAIRCONF_SCHEDULER_MAXPACKETQUEUESIZE]; // Table tournante des paquets
    } packets; // Gestion des paquets non traqués
};

/// Opérations de la queuing discipline
static struct Qdisc_ops hooks_qdisc_ops __read_mostly = {
    .next       = null,
    .cl_ops     = null,
    .id         = FAIRCONF_HOOKS_QDISC_NAME,
    .priv_size  = sizeof(struct hooks_qdiscparam),
    .enqueue    = hooks_qdisc_enqueue,
    .dequeue    = hooks_qdisc_dequeue,
    .peek       = hooks_qdisc_peek,
    .drop       = null,
    .init       = hooks_qdisc_init,
    .reset      = null,
    .destroy    = hooks_qdisc_destroy,
    .change     = null,
    .attach     = null,
    .dump       = null,
    .dump_stats = null,
    .owner      = THIS_MODULE
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Ajoute un paquet en queue de la qdisc.
 * @param param Paramètres de la qdisc concernée
 * @param skb   Socket buffer à mettre en queue
 * @return Précise si le paquet a bien été mis en queue
**/
static inline bool hooks_qdisc_nt_push(struct hooks_qdiscparam* param, struct sk_buff* skb) {
    if (param->packets.count >= FAIRCONF_SCHEDULER_MAXPACKETQUEUESIZE) // Plus de place
        return false;
    param->packets.table[(param->packets.pos + (param->packets.count++)) % FAIRCONF_SCHEDULER_MAXPACKETQUEUESIZE] = skb;
    return true;
}

/** Récupère le plus ancien paquet de la qdisc.
 * @param param Paramètres de la qdisc concernée
 * @return Plus ancien socket buffer
**/
static inline struct sk_buff* hooks_qdisc_nt_peek(struct hooks_qdiscparam* param) {
    return param->packets.table[param->packets.pos]; // Récupération du plus ancien paquet
}

/** Récupère et retire le plus ancien paquet de la qdisc.
 * @param param Paramètres de la qdisc concernée
 * @return Plus ancien socket buffer, retiré de la file
**/
static inline struct sk_buff* hooks_qdisc_nt_pop(struct hooks_qdiscparam* param) {
    struct sk_buff* skb;
    if (param->packets.count == 0) // Aucun paquet
        return null;
    skb = hooks_qdisc_nt_peek(param); // Récupération du paquet
    param->packets.pos = (param->packets.pos + 1) % FAIRCONF_SCHEDULER_MAXPACKETQUEUESIZE; // Nouvelle position du plus ancien
    return skb;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur arrivée d'un paquet dans la queue.
 * @param skb   Socket buffer en attente d'envoi
 * @param qdisc Queuing discipline concerncée
 * @return Statut de la mise en file
**/
static int hooks_qdisc_enqueue(struct sk_buff* skb, struct Qdisc* qdisc) {
    struct nf_conn* nfct = (struct nf_conn*) skb->nfct; // Structure de la connexion dans netfilter
    if (unlikely(!nfct)) { // Cas paquet non traqué (ARP, ...)
        if (unlikely(!hooks_qdisc_nt_push(qdisc_priv(qdisc), skb))) // Mise en file
            return NET_XMIT_DROP;
        return NET_XMIT_SUCCESS;
    }
    if (!scheduler_interface_enqueue(skb, nfct)) // Mise en queue
        return NET_XMIT_DROP;
    return NET_XMIT_SUCCESS;
}

/** Sur peek du paquet à envoyer.
 * @param qdisc Queuing discipline concernée
**/
static struct sk_buff* hooks_qdisc_peek(struct Qdisc* qdisc) {
    struct sk_buff* skb = hooks_qdisc_nt_peek(qdisc_priv(qdisc)); // Socket buffer à retourner
    if (skb) // Au moins un paquet non traqué, prioritaire à l'envoi
        return skb;
    { // Récupération dans un des routeurs associés
        nint count; // Nombre de routeur prêts
        struct netdev* netdev = ((struct hooks_qdiscparam*) qdisc_priv(qdisc))->netdev;
        if (unlikely(!netdev_lock(netdev))) /// LOCK
            return null;
        count = netdev->count;
        if (count == 0) { // Aucun routeur présent
            netdev_unlock(netdev); /// UNLOCK
            return null;
        }
        for (;;) { // Pour tous les routeurs
            struct router* router = list_first_entry_or_null(&(netdev->rdlist), struct router, list); // Routeur en cours
            if (unlikely(!router)) { // Plus aucun routeur présent
                netdev_unlock(netdev); /// UNLOCK
                return null;
            }
            list_rotate_left(&(netdev->rdlist)); // Routeur traité passe en dernier
            router_ref(router); /// REF
            netdev_unlock(netdev); /// UNLOCK
            skb = scheduler_interface_peek(router); // Peek du routeur
            router_unref(router); /// UNREF
            if (skb) // Socket buffer trouvé
                return skb;
            if (count <= 1) // Fin de traitement
                return null;
            count--;
            if (unlikely(!netdev_lock(netdev))) /// LOCK
                return null;
        }
    }
}

/** Sur sortie du paquet à envoyer.
 * @param qdisc Queuing discipline concerncée
 * @return Socket buffer à envoyer sur l'interface
**/
static struct sk_buff* hooks_qdisc_dequeue(struct Qdisc* qdisc) {
    struct sk_buff* skb = hooks_qdisc_nt_pop(qdisc_priv(qdisc)); // Socket buffer à retourner
    if (skb) // Au moins un paquet non traqué, prioritaire à l'envoi
        return skb;
    { // Récupération dans un des routeurs associés
        nint count; // Nombre de routeur prêts
        struct netdev* netdev = ((struct hooks_qdiscparam*) qdisc_priv(qdisc))->netdev;
        if (unlikely(!netdev_lock(netdev))) /// LOCK
            return null;
        count = netdev->count;
        if (count == 0) { // Aucun routeur présent
            netdev_unlock(netdev); /// UNLOCK
            return null;
        }
        for (;;) { // Pour tous les routeurs
            struct router* router = list_first_entry_or_null(&(netdev->rdlist), struct router, list); // Routeur en cours
            if (unlikely(!router)) { // Plus aucun routeur présent
                netdev_unlock(netdev); /// UNLOCK
                return null;
            }
            list_rotate_left(&(netdev->rdlist)); // Routeur traité passe en dernier
            router_ref(router); /// REF
            netdev_unlock(netdev); /// UNLOCK
            skb = scheduler_interface_dequeue(router); // Dequeue du routeur
            router_unref(router); /// UNREF
            if (skb) // Socket buffer trouvé
                return skb;
            if (count <= 1) // Fin de traitement
                return null;
            count--;
            if (unlikely(!netdev_lock(netdev))) /// LOCK
                return null;
        }
    }
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur attachement à une interface.
 * @param qdisc  Structure de la qdisc
 * @param nlattr Netlink attributes
 * @return Code de retour
**/
static int hooks_qdisc_init(struct Qdisc* qdisc, struct nlattr* nlattr) {
    struct hooks_qdiscparam* param = (struct hooks_qdiscparam*) qdisc_priv(qdisc);
    { // Récupération de la netdev
        struct netdev* netdev; // Netdev associée
        struct net_device* net_device; // Network device associée
        spin_lock(&(qdisc->busylock)); /// LOCK
        net_device = qdisc->dev_queue->dev; // Récupération de la netdevice
        dev_hold(net_device); // Compte d'une référence
        spin_unlock(&(qdisc->busylock)); /// UNLOCK
        netdev = scheduler_getnetdev(net_device); // Récupération/allocation de la netdev
        dev_put(net_device); // Décompte d'une référence
        if (!netdev)
            return -ENOMEM;
        param->netdev = netdev; // Prise de référence
    }
    return 0; // Succès
}

/** Sur détachement de l'interface.
 * @param qdisc Structure de la qdisc
**/
static void hooks_qdisc_destroy(struct Qdisc* qdisc) {
    struct hooks_qdiscparam* param = (struct hooks_qdiscparam*) qdisc_priv(qdisc);
    netdev_unref(param->netdev); /// UNREF
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Queuing discipline ▔
/// ▁ Initialisation / destruction ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Initialise la partie hooks.
 * @return Code d'erreur
**/
bool hooks_init(void) {
    if (register_qdisc(&hooks_qdisc_ops) < 0) // Queuing discipline
        goto ERR0;
    if (nf_register_hook(&hooks_input_ipv4_ops) < 0) // Hook post-conntrack pre-mangle (IPv4)
        goto ERR1;
    if (nf_register_hook(&hooks_input_ipv6_ops) < 0) // Hook post-conntrack pre-mangle (IPv6)
        goto ERR2;
    if (nf_register_hook(&hooks_forward_ipv4_ops) < 0) // Hook post-routing pre-mangle (IPv4)
        goto ERR3;
    if (nf_register_hook(&hooks_forward_ipv6_ops) < 0) // Hook post-routing pre-mangle (IPv6)
        goto ERR4;
    return true;
    ERR4: nf_unregister_hook(&hooks_forward_ipv4_ops);
    ERR3: nf_unregister_hook(&hooks_input_ipv6_ops);
    ERR2: nf_unregister_hook(&hooks_input_ipv4_ops);
    ERR1: unregister_qdisc(&hooks_qdisc_ops);
    ERR0: return false;
    return true;
}

/** Nettoie la partie hooks.
**/
void hooks_clean(void) {
    nf_unregister_hook(&hooks_forward_ipv6_ops); // Hook post-routing pre-mangle (IPv6)
    nf_unregister_hook(&hooks_forward_ipv4_ops); // Hook post-routing pre-mangle (IPv4)
    nf_unregister_hook(&hooks_input_ipv6_ops); // Hook post-conntrack pre-mangle (IPv6)
    nf_unregister_hook(&hooks_input_ipv4_ops); // Hook post-conntrack pre-mangle (IPv4)
    unregister_qdisc(&hooks_qdisc_ops); // Queuing discipline
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Initialisation / destruction ▔
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
