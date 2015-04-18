/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▁ Documentation ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/**
 * @file   tools.c
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
 * Définit différentes structures de données spécifiques à ce module.
**/

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Documentation ▔
/// ▁ Déclarations ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/// Headers externes
#include <linux/jiffies.h>

/// Headers internes
#include "tools.h"

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Déclarations ▔
/// ▁ Conversions ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Conversion d'une chaîne base 10 en entier non signé.
 * @param text Chaîne base 10
 * @param size Taille du texte, pouvant être terminé par '\0' ou '\n'
 * @param num  Nombre entier non signé
 * @return Précise si l'opération est un succès
**/
bool tools_strtonint(nint8* text, nint size, nint* num) {
    nint value = 0; // Nombre
    nint i; // Compteur
    nint8 car; // Caractère en cours
    for (i = 0; i < size; i++) {
        car = text[i];
        if (car >= '0' && car <= '9') { // Chiffre
            value = value * 10 + car - '0';
        } else if (car == '\0' || car == '\n') { // Fin ducture
            break;
        } else { // Erreur
            return false;
        }
    }
    *num = value;
    return true;
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Conversion MAC vers texte.
 * @param text    Buffer texte d'écriture (17 octets libres)
 * @param address Adresse MAC
 * @return Espace occupé (17 octets)
**/
nint tools_mactostr(nint8* text, nint8* address) {
    nint8 value; // Valeur en cours
    nint digit; // Chiffre en cours
    nint i; // Compteur
    for (i = 0; i < 6; i++) {
        value = address[i];
        digit = value / 16;
        if (digit < 10) {
            *(text++) = digit + '0';
        } else {
            *(text++) = digit + 'a' - 10;
        }
        digit = value % 16;
        if (digit < 10) {
            *(text++) = digit + '0';
        } else {
            *(text++) = digit + 'a' - 10;
        }
        if (i < 5)
            *(text++) = ':';
    }
    return 17;
}

/** Conversion texte vers MAC.
 * @param address Adresse MAC
 * @param text    Buffer texte d'écriture (17 octets)
 * @return Précise si l'opération est un succès
**/
bool tools_strtomac(nint8* address, nint8* text) {
    nint i = 0; // Compteurs
    nint8 car; // Caractère en cours
    nint8 value; // Valeur en cours
    while (true) {
        car = *(text++);
        if (car >= '0' && car <= '9') {
            value = car - '0';
        } else if (car >= 'A' && car <= 'F') {
            value = car + 10 - 'A';
        } else if (car >= 'a' && car <= 'f') {
            value = car + 10 - 'a';
        } else { // Caractère non attendu
            return false;
        }
        value *= 16;
        car = *(text++);
        if (car >= '0' && car <= '9') {
            value += car - '0';
        } else if (car >= 'A' && car <= 'F') {
            value += car + 10 - 'A';
        } else if (car >= 'a' && car <= 'f') {
            value += car + 10 - 'a';
        } else { // Caractère non attendu
            return false;
        }
        address[i] = value;
        if (i < 5) {
            if (*(text++) != ':') // Caractère non attendu
                return false;
            i++;
        } else {
            break;
        }
    }
    return true;
}

/** Conversion IPv4 vers texte.
 * @param text    Buffer texte d'écriture (15 octets libres min)
 * @param address Adresse IPv4 (network order)
 * @return Espace occupé (15 octets max)
**/
nint tools_ipv4tostr(nint8* text, nint8* address) {
    nint8 bloc; // Valeur du bloc
    nint8* cursor = text; // Curseur de texte
    nint pos; // Position en cours
    for (pos = 0; pos < IPV4_SIZE; pos++) {
        bloc = address[pos];
        if (pos != 0) // N'est pas à la première position
            *(cursor++) = '.'; // '.' inter-nombre
        if (bloc == 0) { // Valeur nulle
            *(cursor++) = '0';
        } else { // Conversion en texte
            nint ten = 100; // Masque puissance de 10
            nint value; // Valeur
            for (; ten > 0; ten /= 10) { // Conversion en décimal (sans les premiers zéros)
                if (ten <= bloc) { // Premier chiffre non nul
                    value = bloc / ten;
                    bloc -= value * ten;
                    *(cursor++) = value + '0';
                    ten /= 10;
                    break;
                }
            }
            for (; ten > 0; ten /= 10) { // Conversion en décimal
                value = bloc / ten;
                bloc -= value * ten;
                *(cursor++) = value + '0';
            }
        }
    }
    return (nint) cursor - (nint) text; // Espace occupé
}

/** Conversion texte vers IPv4.
 * @param address Adresse IPv4 (network order)
 * @param text    Buffer texte d'écriture (null-terminated ou new-line-terminated)
 * @return Précise si l'opération est un succès
**/
bool tools_strtoipv4(nint8* address, nint8* text) {
    nint bloc; // Valeur du bloc
    nint8 car; // Caractère en cours
    nint pos = 0; // Position dans l'adresse
    nint i; // Compteur
    LOOP: {
        bloc = 0;
        for (i = 0; i < 4; i++) { // Lecture du bloc et du symbole final
            car = *(text++);
            if (car == '.') { // Fin de bloc
                if (pos >= IPV4_SIZE - 1) // Adresse trop longue
                    return false;
                if (bloc >= 256) // Bloc invalide
                    return false;
                address[pos++] = (nint8) bloc; // Écriture du bloc
                goto LOOP;
            } else if (car == '\0' || car == '\n') { // Fin de chaîne ou nouvelle ligne
                if (pos != IPV4_SIZE - 1) // Adresse trop courte/longue
                    return false;
                address[pos++] = bloc; // Écriture du bloc
                return true;
            } else if (car >= '0' && car <= '9') {
                bloc = bloc * 10 + (car - '0');
            } else { // Caractère invalide
                return false;
            }
        }
        return false; // Dépasse les 3 caractères
    }
}

/** Conversion IPv6 vers texte.
 * @param text    Buffer texte d'écriture (39 octets libres min)
 * @param address Adresse IPv6 (network order)
 * @return Espace occupé (39 octets max)
**/
nint tools_ipv6tostr(nint8* text, nint16* address) {
    nint longzero = (nint) -1; // Position de la plus longue séquence de zéros
    nint longsize = 0; // Taille de la plus longue séquence de zéros
    nint16 bloc; // Valeur du bloc
    nint pos; // Position en cours
    { // Recherche de la plus longue séquence de zéros
        nint clz = 0; // Position en cours
        nint cls = 0; // Taille en cours
        for (pos = 0; pos < IPV6_SIZE / 2; pos++) { // Passage de tous les bloc de deux octets
            bloc = address[pos]; // Lecture du bloc
            if (bloc == 0) { // Bloc nul
                if (cls == 0) // Début de séquence
                    clz = pos; // Positionnement du bloc
                cls++;
            } else {
                if (cls > 1 && cls > longsize) { // On a passé une séquence (au moins 2 blocs) plus grande
                    longzero = clz;
                    longsize = cls;
                }
                cls = 0;
            }
        }
        if (cls > 1 && cls > longsize) { // On a passé une séquence (au moins 2 blocs) plus grande
            longzero = clz;
            longsize = cls;
        }
    }
    { // Conversion en texte
        nint8* cursor = text; // Curseur de texte
        for (pos = 0; pos < IPV6_SIZE / 2; pos++) {
            bloc = address[pos];
            if (pos == longzero) { // Sur la série de zéros
                *(cursor++) = ':'; // ':' inter-bloc
                pos += longsize - 1; // Avancée (-1 à cause de la boucle)
                if (pos == IPV6_SIZE / 2 - 1) { // Fin de chaîne (-1 à cause de la boucle)
                    *(cursor++) = ':'; // ':' final
                    break; // Pour sortie plus rapide
                }
            } else {
                zint i = 3; // Compteur signé
                nint value; // Valeur
                if (pos != 0) // N'est pas à la première position
                    *(cursor++) = ':'; // ':' inter-bloc
                while (i != 0 && (bloc >> (i * 4)) == 0) // Suppression des zéros inutiles
                    i--;
                for (; i >= 0; i--) { // Conversion en hexadécimal
                    value = ((bloc >> (i * 4)) & 0xF);
                    if (value >= 0 && value <= 9) {
                        *(cursor++) = value + '0';
                    } else { // On a forcément 10 <= value <= 15
                        *(cursor++) = (value - 10) + 'a';
                    }
                }
            }
        }
        return (nint) cursor - (nint) text; // Espace occupé
    }
}

/** Conversion texte vers IPv6.
 * @param address Adresse IPv6 (network order)
 * @param text    Buffer texte d'écriture (null-terminated ou new-line-terminated)
 * @return Précise si l'opération est un succès
**/
bool tools_strtoipv6(nint16* address, nint8* text) {
    nint pos = 0; // Position dans l'adresse
    nint stackpos = 0; // Position dans la pile
    nint16 stack[IPV6_SIZE / 2]; // Pile des blocs post-séquence
    bool seqpassed = false; // Précise si une séquence a été trouvée
    nint16 bloc; // Valeur du bloc en cours
    nint8 car; // Caractère en cours
    nint i; // Compteur
    if (*text == ':') // Commence par ':' et non par un bloc simple
        text++; // Saut du premier ':', nécessaire à l'algorithme
    LOOP: {
        bloc = 0; // Initialisation de la valeur du bloc
        for (i = 0; i < 5; i++) { // Lecture du bloc et du symbole final
            car = *(text++); // Caractère en cours
            if (car == ':') { // Fin du bloc
                if (i == 0) { // Séquence de zéros
                    if (seqpassed) // Séquence déjà passé
                        return false;
                    seqpassed = true;
                    goto LOOP;
                } else {
                    if (pos + stackpos >= IPV6_SIZE / 2 - 1) // Adresse trop longue
                        return false;
                    if (seqpassed) { // Post-séquence
                        stack[stackpos++] = bloc; // Écriture du bloc
                    } else { // Pré-séquence
                        address[pos++] = bloc; // Écriture du bloc
                    }
                    goto LOOP;
                }
            } else if (car == '\0' || car == '\n') { // Fin de chaîne ou nouvelle ligne
                if (seqpassed) { // Post-séquence
                    stack[stackpos++] = bloc; // Écriture du bloc
                } else { // Pré-séquence
                    address[pos++] = bloc; // Écriture du bloc
                }
                goto DONE;
            } else if (car >= '0' && car <= '9') {
                bloc = bloc * 16 + (car - '0');
            } else if (car >= 'a' && car <= 'f') {
                bloc = bloc * 16 + (car + 10 - 'a');
            } else if (car >= 'A' && car <= 'F') {
                bloc = bloc * 16 + (car + 10 - 'A');
            } else { // Caractère invalide
                return false;
            }
        }
        return false; // Dépasse les quatres caractères
    }
    DONE: if (seqpassed) { // Séquence trouvée, pile à déplacer
        while (pos < IPV6_SIZE / 2 - stackpos)
            address[pos++] = 0;
        stackpos = 0; // Pour compteur
        while (pos < IPV6_SIZE / 2)
            address[pos++] = stack[stackpos++];
    }
    return true;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Conversions ▔
/// ▁ Liste ordonnée ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Signal dans la section puis les sections parentes qu'un élément est disponible.
 * @param section Section cible
 * @param offset  Offset "à signaler"
**/
static as(hot) void sortlist_signal(struct sortlist_header* header, nint offset) {
    SORTLIST_TYPE used; // Variable tampon
    while (true) { // Sur tous les étages
        used = header->used;
        header->used |= ((SORTLIST_TYPE) 1) << offset; // Ajout du bit
        if (used == 0 && header->parent) { // Header désormais non vide, signalement sur le header parent
            offset = header->offset;
            header = &(header->parent->header);
        } else { // Sinon fin
            break;
        }
    }
}

/** Efface les signaux précédemment levés.
 * @param section Section cible
 * @param offset  Offset "à effacer"
**/
static as(hot) void sortlist_clear(struct sortlist_header* header, nint offset) {
    while (true) { // Sur tous les étages
        header->used &= ~(((SORTLIST_TYPE) 1) << offset); // Suppression du bit
        if (header->used == 0 && header->parent) { // Header désormais vide, effacement sur le header parent
            offset = header->offset;
            header = &(header->parent->header);
        } else { // Sinon fin
            break;
        }
    }
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/// FIXME: Prise en charge de plus d'architectures et de configurations

/** Obtient la position du premier bit non nul d'un masque.
 * @param mask Masque à traiter, doit être non nul
 * @return Position de premier bit
**/
static inline as(hot) nint sortlist_used(SORTLIST_TYPE mask) {
#if FAIRCONF_TARGETARCH_x86_64 == 1 && defined FAIRCONF_SORTLIST_PRECISION_64
	register nint pos;
	asm (
		"bsfq %[src], %[dst];" // Récupération de l'offset
	: [dst] "=r" (pos) : [src] "r" (mask));
    return pos;
#else
	nint pos = 0;
	while (!(mask & 1)) { // Recherche de la position du premier 1
        mask >>= 1;
        pos++;
	}
	return pos;
#endif
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Initialise la structure.
 * @param list Liste à initialiser
**/
void sortlist_init(struct sortlist* list) {
    struct sortlist_index* parent = null; // Index parent en cours
    { // Initialisation de la structure
        list->phase = 0;
        list->current = list->sections;
    }
    if (SORTLIST_INDEX) { // Initialisation des index
        nint position = 0; // Position du header
        nint offset = 0; // Offset du bit dans l'index parent
        struct sortlist_index* current = list->index; // Index en cours
        { // Initialisation du premier index
            current->header.parent = null;
            current->header.used = 0;
            current->header.offset = 0;
            parent = current; // Premier index
            current++; // Index suivant
        }
        while (position < SORTLIST_INDEX - 1) { // Pour tous les autres index
            { // Initialisation parent/enfant
                current->header.parent = parent;
                current->header.used = 0;
                current->header.offset = offset;
                parent->children[offset].index = current;
            }
            current++; // Index suivant
            position++; // Position suivante
            if (position >= SORTLIST_INDEX - 1) // Fin de traitement
                break;
            offset = position % SORTLIST_SIZE; // Calcul de l'offset du courant pour le parent
            if (offset == 0) // Nouveau parent
                parent++; // Avancée du parent
        }
    }
    { // Initialisation des sections (toujours au moins une)
        nint position = 0; // Position du header
        nint offset = 0; // Offset du bit dans l'index parent
        struct sortlist_section* current = list->sections; // Index en cours
        while (true) { // Pour tous les index (condition d'entrée position < SORTLIST_SECTIONS toujours vraie)
            { // Initialisation parent/enfant
                nint i; // Compteur
                current->header.parent = parent;
                current->header.used = 0;
                current->header.offset = offset;
                parent->children[offset].section = current;
                for (i = 0; i < SORTLIST_SIZE; i++) // Initialisation des chaînes de la section
                    INIT_LIST_HEAD(current->objects + i);
            }
            current++; // Section suivante
            position++; // Position suivante
            if (position >= SORTLIST_SECTIONS) // Fin de traitement
                break;
            offset = position % SORTLIST_SIZE; // Calcul de l'offset du courant pour le parent
            if (offset == 0) // Nouveau parent
                parent++; // Avancée du parent
        }
    }
}

/** Ajoute un objet dans une liste ordonnée.
 * @param list   Liste ordonnée cible
 * @param object list_head de l'objet à ajouter
 * @param retard Retard de l'élément à ajouter
 * @return Précise si l'élément a été ajouté sans diminution du retard
**/
bool as(hot) sortlist_push(struct sortlist* list, struct list_head* object, nint retard) {
    bool exact; // Valeur du retour
    if (retard >= SORTLIST_LENGTH) { // Retard trop important
        retard = SORTLIST_LENGTH - 1; // On fait "saturer"
        exact = false; // Et on le précise
    } else {
        exact = true;
    }
    { // Ajout de l'objet
        nint offset = (list->phase + retard) % SORTLIST_LENGTH; // Offset de l'objet
        struct sortlist_section* section = list->sections + offset / SORTLIST_SIZE; // Section concernée
        offset %= SORTLIST_SIZE; // Rammené à la section en cours
        { // Ajout et signalement
            struct list_head* head = section->objects + offset; // Objet concerné dans la section
            if (list_empty(head)) { // Liste vide
                sortlist_signal(&(section->header), offset); // Signalement
            }
            list_add_tail(object, head); // Ajout de l'objet
        }
    }
    return exact;
}

/** Obtient l'élément le mieux classé de la liste ordonnée.
 * @param list Liste ordonnée cible
 * @return Pointeur sur la tête de liste de l'élément le moins retardé (null si aucun élément)
**/
struct list_head* as(hot) sortlist_get(struct sortlist* list) {
    if (list->index->header.used == 0) // Aucun objet (valide même si aucun index car alors list->index = list->section)
        return null;
    { // Obtention du mieux classé
        struct sortlist_header* header = (struct sortlist_header*) list->current; // Header en cours
        SORTLIST_TYPE mask; // Masque utilisé
        nint phase = list->phase; // Nouvelle phase
        nint offset = list->phase % SORTLIST_SIZE; // Offset sur le header en cours
        nint jump = 1; // Nombre d'élements "représentés" par un bit
        while (true) { // Recherche dans l'arbre
            mask = header->used >> offset; // Masque considéré à partir du bit offset
            if (mask == 0) { // On monte (peut-être)
                phase += (SORTLIST_SIZE - offset) * jump; // Décompte du nombre d'emplacements sautés
                if (header->parent) { // N'est pas la racine, on monte
                    jump *= SORTLIST_SIZE; // Un bit "représentera" size fois plus d'éléments
                    offset = header->offset;
                    header = (struct sortlist_header*) header->parent;
                } else { // Est la racine, on reboucle (il y a forcément au moins un élément libre)
                    offset = 0; // Retour à...
                    phase = 0; // ...l'origine
                }
            } else { // On descent (peut-être)
                nint pos = sortlist_used(mask); // On a forcément un élément
                phase += pos * jump; // Décompte du nombre d'emplacements sautés
                if (jump == 1) { // Niveau section, on enregistre la position et on sort
                    list->phase = phase;
                    list->current = (struct sortlist_section*) header;
                    return (((struct sortlist_section*) header)->objects + (phase % SORTLIST_SIZE))->next; // Objet le mieux classé
                } else { // Niveau index, on descent
                    header = ((struct sortlist_index*) header)->children[offset + pos].header;
                    offset = 0; // Car descente sur un index/une section nouvelle
                    jump /= SORTLIST_SIZE; // Un bit "représentera" size fois moins d'éléments
                }
            }
        }
    }
}

/** Retire l'élément le mieux classé de la liste ordonnée.
 * @param list Liste ordonnée cible
 * @return Pointeur sur la tête de liste de l'élément le moins retardé (null si aucun élément)
**/
struct list_head* as(hot) sortlist_pop(struct sortlist* list) {
    struct list_head* object = sortlist_get(list); // Récupération de l'élément le mieux classé
    if (!object) // Aucun élément dans la liste
        return null;
    { // Retrait de l'objet
        nint offset = list->phase % SORTLIST_SIZE; // Offset par rapport à la section en cours
        list_del(object); // Retrait de l'objet
        if (list_empty(list->current->objects + offset)) { // Liste désormais vide
            sortlist_clear(&(list->current->header), offset); // Effacement dans les sections
        }
    }
    return object;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Liste ordonnée ▔
/// ▁ Débit moyen glissant ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Effectue une rotation des blocs.
 * @param tput Structure de calcul du débit
 * @return Position en cours
**/
static as(hot) nint throughput_rotate(struct throughput* tput) {
    nint64 delta = time_now() - tput->zero; // Différence de temps depuis le début
    nint head = delta / THROUGHPUT_DELTA + 1; // Position du premier bloc non utilisé
    if (head != tput->head) { // Rotation possible
        nint tail = tput->tail; // Queue actuelle
        nint count = head - tail; // Nombre de blocs concernés
        if (count > THROUGHPUT_PRECISION) { // Rotation nécessaire
            nint i; // Compteur
            nint pos; // Position
            nint sum = tput->sum; // Somme en cours
            count = head - THROUGHPUT_PRECISION; // Nouvelle queue (réutilisation de la variable count)
            for (i = tail; i < count; i++) { // Élimination des blocs
                pos = i % THROUGHPUT_PRECISION;
                sum -= tput->values[pos];
                tput->values[pos] = 0;
            }
            tput->sum = sum; // Nouvelle somme
            tput->tail = count; // Nouvelle queue (variable substituée)
        }
        tput->head = head;
    }
    tput->time = (delta <= THROUGHPUT_BASE ? delta : THROUGHPUT_BASE + delta % THROUGHPUT_DELTA); // Temps écoulé, avant ou après la première fenêtre
    tput->last = delta; // Date de dernière rotation
    return (head - 1) % THROUGHPUT_PRECISION; // (Nouvelle) première position utilisée
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Initialise la structure.
 * @param tput Structure de calcul du débit
**/
void throughput_init(struct throughput* tput) {
    nint i; // Compteur
    tput->head = 1;
    tput->tail = 0;
    tput->zero = time_now();
    tput->last = 0;
    tput->sum = 0;
    tput->time = 0;
    for (i = 0; i < THROUGHPUT_PRECISION; i++)
        tput->values[i] = 0;
}

/** Ajoute une valeur.
 * @param tput Structure de calcul du débit
 * @param value Valeur à ajouter
**/
void as(hot) throughput_add(struct throughput* tput, zint value) {
    nint pos = throughput_rotate(tput); // Rotation
    tput->values[pos] += value;
    tput->sum += value;
}

/** Calcul la moyenne.
 * @param tput Structure de calcul du débit
 * @return Moyenne sur la période donnée
**/
zint as(hot) throughput_get(struct throughput* tput) {
    throughput_rotate(tput); // Rotation
    return tput->time == 0 ? 0 : tput->sum / tput->time;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Débit moyen glissant ▔
/// ▁ Moyenne glissante ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

/** Effectue une rotation des blocs.
 * @param avrg Structure de calcul de la moyenne
 * @return Position en cours
**/
static as(hot) nint average_rotate(struct average* avrg) {
    nint64 delta = time_now() - avrg->zero; // Différence de temps depuis le début
    nint head = delta / THROUGHPUT_DELTA + 1; // Position du premier bloc non utilisé
    if (head != avrg->head) { // Rotation possible
        nint tail = avrg->tail; // Queue actuelle
        nint count = head - tail; // Nombre de blocs concernés
        if (count > AVERAGE_PRECISION) { // Rotation nécessaire
            nint i; // Compteur
            nint pos; // Position
            zint sum = avrg->sum; // Somme en cours
            nint cnt = avrg->cnt; // Nombre de valeurs en cours
            count = head - AVERAGE_PRECISION; // Nouvelle queue (réutilisation de la variable count)
            for (i = tail; i < count; i++) { // Élimination des blocs
                pos = i % AVERAGE_PRECISION;
                sum -= avrg->values[pos];
                cnt -= avrg->counts[pos];
                avrg->values[pos] = 0;
                avrg->counts[pos] = 0;
            }
            avrg->sum = sum; // Nouvelle somme
            avrg->cnt = cnt; // Nouveau nombre de valeurs
            avrg->tail = count; // Nouvelle queue (variable substituée)
        }
        avrg->head = head; // (Nouvelle) tête
    }
    avrg->last = delta; // Date de dernière rotation
    return (head - 1) % AVERAGE_PRECISION; // (Nouvelle) première position utilisée
}

/// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Initialise la structure.
 * @param avrg Structure de calcul de la moyenne
**/
void average_init(struct average* avrg) {
    nint i; // Compteur
    avrg->head = 1;
    avrg->tail = 0;
    avrg->zero = time_now();
    avrg->last = 0;
    avrg->sum = 0;
    avrg->cnt = 0;
    for (i = 0; i < AVERAGE_PRECISION; i++) {
        avrg->values[i] = 0;
        avrg->counts[i] = 0;
    }
}

/** Ajoute une valeur.
 * @param avrg Structure de calcul de la moyenne
 * @param value Valeur à ajouter
**/
void as(hot) average_add(struct average* avrg, zint value) {
    nint pos = average_rotate(avrg); // Rotation
    avrg->values[pos] += value;
    avrg->counts[pos]++;
    avrg->sum += value;
    avrg->cnt++;
}

/** Calcul la moyenne.
 * @param avrg Structure de calcul de la moyenne
 * @return Moyenne sur la période donnée
**/
zint as(hot) average_get(struct average* avrg) {
    average_rotate(avrg); // Rotation
    return avrg->cnt == 0 ? 0 : avrg->sum / avrg->cnt;
}

/// ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔ Moyenne glissante ▔
/// ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
