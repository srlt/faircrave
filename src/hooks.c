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

/// Headers internes
#include "hooks.h"

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
    if (ctinfo == IP_CT_NEW) { // Est une nouvelle connexion
        if (!scheduler_interface_input(skb, ct, version)) // Mark refusée
            return NF_DROP;
    }
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
    if (ctinfo == IP_CT_NEW) { // Est une nouvelle connexion
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
static struct sk_buff* hooks_qdisc_dequeue(struct Qdisc*);
static struct sk_buff* hooks_qdisc_peek(struct Qdisc*);

static int hooks_qdisc_init(struct Qdisc*, struct nlattr*);
static void hooks_qdisc_reset(struct Qdisc*);
static void hooks_qdisc_destroy(struct Qdisc*);
static int hooks_qdisc_change(struct Qdisc*, struct nlattr*);

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// TODO: Configuration des routeurs cibles d'une qdisc par l'administrateur

struct hooks_qdiscparam {
    nint zero; // Paramètre inutilisé for now
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
    .reset      = hooks_qdisc_reset,
    .destroy    = hooks_qdisc_destroy,
    .change     = hooks_qdisc_change,
    .attach     = null,
    .dump       = null,
    .dump_stats = null,
    .owner      = THIS_MODULE
};

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur arrivée d'un paquet dans la queue.
 * @param skb   Socket buffer en attente d'envoi
 * @param qdisc Queuing discipline concerncée
 * @return Statut de la mise en file
**/
static int hooks_qdisc_enqueue(struct sk_buff* skb, struct Qdisc* qdisc) {
log(KERN_DEBUG, "skb = %p, qdiscparam = %p", skb, qdisc_priv(qdisc));
    /// TODO: Enqueue de la qdisc
    return NET_XMIT_DROP; // Temporaire
}

/** Sur sortie du paquet à envoyer.
 * @param qdisc Queuing discipline concerncée
 * @return Socket buffer à envoyer sur l'interface
**/
static struct sk_buff* hooks_qdisc_dequeue(struct Qdisc* qdisc) {
log(KERN_DEBUG, "qdiscparam = %p", qdisc_priv(qdisc));
    /*struct sk_buff* skb = hooks_qdisc_peek(qdisc); // Paquet
    if (skb) { // Paquet présent
        /// TODO: Sortie du paquet de la file
    }
    return skb;*/
    return null;
}

/** Sur peek du paquet à envoyer.
 * @param qdisc Queuing discipline concernée
**/
static struct sk_buff* hooks_qdisc_peek(struct Qdisc* qdisc) {
log(KERN_DEBUG, "qdiscparam = %p", qdisc_priv(qdisc));
    /*struct sk_buff* skb; // Paquet
    /// TODO: Peek du paquet (queuing discipline)
    return skb;*/
    return null;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Sur attachement à une interface.
 * @param qdisc  Structure de la qdisc
 * @param nlattr Netlink attributes
 * @return Code de retour
**/
static int hooks_qdisc_init(struct Qdisc* qdisc, struct nlattr* nlattr) {
log(KERN_DEBUG, "qdiscparam = %p, nlattr = %p", qdisc_priv(qdisc), nlattr);
    /// TODO: Initialisation de la queuing discipline
    return 0; // Succès
}

/** Sur reset de la file des paquets.
 * @param qdisc Queuing discipline concernée
**/
static void hooks_qdisc_reset(struct Qdisc* qdisc) {
log(KERN_DEBUG, "qdiscparam = %p", qdisc_priv(qdisc));
    /// TODO: Opérations sur reset de la queuing discipline
}

/** Sur détachement de l'interface.
 * @param qdisc Structure de la qdisc
**/
static void hooks_qdisc_destroy(struct Qdisc* qdisc) {
log(KERN_DEBUG, "qdiscparam = %p", qdisc_priv(qdisc));
    /// TODO: Opérations de détachement de la queuing discipline
}

/** Sur changement des paramètres de la qdisc.
 * @param qdisc Structure de la qdisc
 * @param nlattr Netlink attributes
 * @return Code de retour
**/
static int hooks_qdisc_change(struct Qdisc* qdisc, struct nlattr* nlattr) {
log(KERN_DEBUG, "qdiscparam = %p, nlattr = %p", qdisc_priv(qdisc), nlattr);
    /// TODO: Opérations de détachement de la queuing discipline
    return 0; // Succès
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
