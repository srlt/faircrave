# FairCrave

Module visant un partage équitable du débit montant offert par N routeurs entre M utilisateurs, appelés "adhérents".
Chaque utilisateur peut avoir et utiliser un nombre libre de machines.

Le module réalise également le load-balancing des nouvelles connexions entre les routeurs.

D'autres fonctionnalités mineures, ou expérimentales, sont également présentes.

Il s'agit d'une partie d'un projet étudiant, répondant aux besoins de l'association *Supélec Rézo Rennes*.

# Compilation

Ce module est écrit pour la version 3.14 du noyau Linux.

Un fichier de configuration pour la version 3.14.33 se trouve dans le répertoire misc/.

Pour compiler le module, depuis src/ :

> $ make module

Le binaire (faircrave.ko) sera déplacé dans bin/.

# Vue d'ensemble

*TODO: Vue d'ensemble du fonctionnement du module*

# Utilisation

### Interface avec l'administrateur

Le module crée une arborescence dans le sysfs, dans /sys/kernel/faircrave/ :

Arborescence       | Accès | Brève description
------------------ | ----- | -----------------
connections        | R-    | Logs de connexions (limité, à lire régulièrement), *si compilé*
create_member      | -W    | Crée un nouvel adhérent avec pour ID le nombre écrit
create_router      | -W    | Crée un nouveau routeur avec pour ID le nombre écrit
default_member     | RW    | ID de l'adhérent par défaut, *optionnel*
delete_member      | -W    | Supprime l'adhérent ayant pour ID le nombre écrit
delete_router      | -W    | Supprime le routeur ayant pour ID le nombre écrit
maxconnperuser     | RW    | Nombre maximal de connexion qu'un adhérent peut utiliser simultanément
maxlogentries      | RW    | Nombre maximal de logs stockés par le module
members/           |       | Liste des adhérents
    `<ID>`/        |       | Informations relatives à l'adhérent ID
        latency    | R-    | Latence (en ms) observée à travers le module pour cet adhérent
        maxlatency | RW    | Latence (en ms) maximale demandée pour l'UDP (expérimental)
        priority   | RW    | Priorité de l'adhérent (plus faible = plus prioritaire)
        router     | RW    | ID du routeur préféré (nouvelles connexions sur ce routeur), *optionnel*
        tput_ask   | R-    | Débit (en ko/s) total entrant dans le scheduler
        tput_down  | R-    | Débit (en ko/s) total revenant vers l'adhérent
        tput_lost  | R-    | Débit (en ko/s) total perdu par le scheduler
        tput_up    | R-    | Débit (en ko/s) total sortant du scheduler
        tput_up    | RW    | Liste des tuples MAC/IPv4 ou MAC/IPv6 de l'adhérent
routers/           |       | Liste des routeurs
    `<ID>`/        |       | Informations relatives au routeur ID
        allow_ipv4 | RW    | Le routeur supporte IPv4
        allow_ipv6 | RW    | Le routeur supporte IPv6
        latency    | R-    | RTT (en ms) moyen à travers ce routeur, *inutilisé*
        status     | RW    | État du routeur (online, offline, closing)
        tput_down  | R-    | Débit (en ko/s) descendant de ce routeur
        tput_limit | RW    | Débit (en ko/s) montant maximal vers ce routeur
        tput_up    | R-    | Débit (en ko/s) montant vers ce routeur

*TODO: Détails sur les éléments ci-dessus*

### Exemples

*TODO: Cas d'usage et exemples de configuration*

