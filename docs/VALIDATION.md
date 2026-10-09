# Validation - 9 octobre 2026

Ce compte rendu distingue les vérifications réellement exécutées de la validation du montage final. Le projet ne doit pas être présenté comme intégralement validé tant que les points restants ci-dessous ne sont pas résolus.

## Tests logiciels exécutés

`./scripts/projet.sh test` compile le code C du projet sur le poste avec AddressSanitizer et UndefinedBehaviorSanitizer. Quatre programmes ont terminé avec succès :

1. **Cœur** : domaines des commandes, chaînes invalides, débordements de nombres, paramètres inchangés après refus, dates bissextiles, conversion jour, transitions de modes, mille insertions circulaires aux limites d'int16, conservation de l'historique en erreur, remise à zéro des compteurs, checksum GPS, champs vides, coordonnées hors domaine et hémisphères.
2. **SD** : vrai FatFs et vrai `sd_files.c`, avec secteurs servis par une image FAT16 de test de 16 Mo. Ajout, archivage avant dépassement, révision supérieure à neuf, démontage/remontage, erreur de secteurs et carte pleine. Aucun accès à la SD physique n'est effectué par ce test.
3. **Station** : vrai `station.c` exécuté avec des périphériques et une horloge simulés. Pas d'acquisition en configuration, commandes EEPROM, refus si la sauvegarde échoue, sortie après trente minutes, arrêt complet des écritures en maintenance, retour au mode antérieur, intervalle doublé et GPS un cycle sur deux.
4. **EEPROM** : vrai `eeprom.c` avec transactions I²C simulées. Pages de 32 octets, choix de la dernière copie valide, corruption CRC et coupure simulée après chacun des octets d'une sauvegarde. L'ancienne configuration reste lisible en cas d'échec.

Trace : `verification/tests.txt`. Les valeurs et coordonnées des tests sont des jeux de données de test ; elles ne proviennent pas de la position de la carte.

## Compilation ARM exécutée

`pio run -e release -e debug -e diagnostic -e diagnostic_sd_grove` : quatre profils compilés avec succès, sans avertissement C dans la dernière trace.

| Profil | RAM statique annoncée | Flash annoncée |
|---|---:|---:|
| Release | 5288 octets / 98304 | 61276 octets / 1048576 |
| Debug | 5332 octets / 98304 | 92056 octets / 1048576 |

Ces chiffres correspondent aux dernières sources compilées lors de la rédaction. La RAM annoncée ne mesure pas le pic de pile ni les allocations internes des bibliothèques C. L'application n'appelle pas `malloc`, mais les bibliothèques C liées, notamment la conversion de texte en nombre, peuvent utiliser le tas.

Traces : `verification/compilation-montage-grove.txt`, `verification/memoire-release.txt`. Les cadres estimés par GCC sont filtrés sur les fonctions encore présentes dans l'ELF. Le pic de pile avec les appels de bibliothèque et les interruptions reste à mesurer physiquement.

## Vérifications sur la Nucleo

- Sonde ST-LINK/V2.1 détectée, cible STM32L476, tension voisine de 3,23 V.
- Sauvegarde complète de la Flash antérieure réalisée localement dans `Documents/3ww-sauvegardes/firmware-avant-3ww.bin`, avant tout téléversement ; cette sauvegarde personnelle n'est pas publiée.
- Programme de diagnostic compilé, téléversé et vérifié par ST-LINK.
- Scan I²C : `0x76` et `0x51`. Identifiant météo `0x60` : BME280. Victor confirme BME280 et RTC Grove v2 PCF85063TP. Pilotes adaptés à ces références.
- Trames GPS reçues physiquement sur A0. Les trames observées indiquent un fix invalide et aucun satellite ; aucune position valide n'a encore été démontrée.
- Le débogueur a confirmé `SystemCoreClock=80000000`, l'initialisation du BME280 et l'absence de faute processeur dans la capture examinée.
- La connexion USB a initialement échoué avec le montage complet, puis a fonctionné avec la Nucleo seule. Après ajout du support SD, la sortie série est devenue silencieuse ; le montage est en cours de vérification.
- Victor confirme un Seeed Studio SD Card Shield v4.4, daté du 04/01/2022. Les signaux attendus sont D13/D12/D11 et CS sur D4 ; la notice de la famille V4 prévoit leur passage par les connecteurs Arduino. Aucun fil supplémentaire n'est nécessaire si cet empilement relie bien les broches. L'ordre et l'alignement des cartes restent à vérifier ; l'accès aux fichiers sur la SD réelle n'est pas encore validé.
- Vérification de l'archive Eagle officielle Seeed v4.3 : les sélecteurs DI/DO/CLK choisissent entre ICSP et D11/D12/D13. Le routage effectif sur la v4.4 n'est pas encore identifié. L'empilement ne suffit pas à prouver que les trois fils prévus par CESI sont inutiles ; une inspection physique est nécessaire.
- Nouvel essai Nucleo → SD → Grove : erreur USB Linux `-71`, sans port série ni accès OpenOCD. Même erreur avec uniquement Nucleo → SD, carte mémoire retirée. Après retrait du shield SD, la Nucleo nue retrouve `/dev/ttyACM0` et l'accès ST-LINK. Ce contrôle situe le problème dans la présence du shield SD ou son montage, sans déterminer à lui seul la cause électrique exacte.
- Programme complet Release téléversé sur la Nucleo nue et Flash vérifiée (`Verified OK`). Console : après une fin de ligne pour resynchroniser la réception, `VERSION` répond `3WW 1.0.0 LOT=CESI-2026 BUILD=1`. Le programme complet remplace désormais le diagnostic sur la carte.
- Photos du dessus et du dessous du support reçues : connecteur ICSP bleu à six trous identifié, ainsi que les sélecteurs DI/DO/CLK. Les trois positions SPI sont repérées dans la documentation technique à partir du fichier Eagle officiel et de l'orientation des photos. La continuité électrique et la cause de l'échec USB ne peuvent pas être prouvées par ces photos.
- Prochain essai demandé : support SD séparé de la Nucleo, sans carte mémoire, avec seulement les fils 5V et GND entre les broches portant ces inscriptions. Si l'USB fonctionne dans cette configuration, ajout ultérieur de CS et des trois signaux SPI. Aucun résultat de cet essai n'est encore disponible.
- Victor indique d'abord ne pas avoir de fils Dupont. L'essai séparé proposé avec des fils mâle-mâle n'est donc pas effectué. Il précise que le connecteur ICSP SD s'emboîte sur son Grove. Vérification du schéma officiel Grove Base Shield v2 : ce modèle ne relie pas les contacts ICSP MOSI/MISO/SCK aux nets D11/D12/D13. Aucune liaison SPI fonctionnelle n'a été démontrée à cette étape.
- Une vérification des capteurs sans support SD a été proposée : BME280 et RTC sur I²C, GPS sur A0, UART Grove libre. Ce remontage n'est pas encore confirmé.
- Victor confirme ensuite un Grove Base Shield **v2.1**. L'archive PCB officielle proposée sur la page Seeed est marquée **v2.0, 03/10/2014**. Elle ne constitue pas une preuve du routage de la v2.1 : la continuité ICSP vers D11/D12/D13 sur son exemplaire reste à vérifier. Le nom du shield Grove est maintenant identifié.
- Une nouvelle photo et sa confirmation identifient quatre câbles Grove vers quatre Dupont **femelles**. Ces prises peuvent recevoir des contacts mâles accessibles ; il n'est donc plus nécessaire d'attendre des fils mâle-mâle pour préparer un essai spécifique. Le diagnostic séparé utilise SPI1 sur PB3/PB4/PB5 (D3/D5/D4) et CS sur PA10 (D2), conformément aux fonctions alternatives du STM32L476. Le programme complet conserve son brochage initial.
- Après confirmation du montage Nucleo → Grove seul, modules débranchés, `diagnostic_sd_grove` est téléversé et **Verified OK** par ST-LINK (tension mesurée par la sonde : environ 3,23 V). La console est silencieuse pendant cette capture ; lecture des variables par GDB : `diag_sd_init=false`, `diag_sd_statut=1` (absence de réponse CMD0), programme dans sa boucle finale `HAL_Delay`. Le lecteur n'est pas raccordé, donc ce résultat est attendu et ne valide pas la SD.
- Les profils `release`, `debug` et `diagnostic_sd_grove` compilent sans avertissement C après correction de la dépendance de calendrier utilisée par FatFs. Log : `verification/compilation-sd-grove.txt`. Le diagnostic occupe 1 392 octets de RAM statique et 27 156 octets de Flash. Il crée un fichier de test libre, écrit puis relit 13 octets ; aucun résultat physique d'écriture/relecture n'est encore disponible.
- Contrôle d'alimentation séparée confirmé par Victor : lecteur sans carte mémoire, alimentation seule via son port Grove I²C (5V fixe), rouge vers Nucleo CN7-18 (+5V), noir vers CN7-20 (GND). Le port `/dev/ttyACM0` est présent et OpenOCD examine correctement la cible STM32L476, avec une tension cible de 3,240 V. Cette connexion fonctionne donc dans la capture avec le lecteur alimenté à côté ; cela ne démontre pas encore les communications SPI ni la cause exacte de la panne lors de l'empilement.
- Victor confirme ensuite les quatre signaux CS/SCK/MOSI/MISO raccordés selon la procédure, carte mémoire insérée et USB ST-LINK rebranché. Linux détecte l'identité ST-LINK, mais échoue à configurer l'USB (`can't set config #1, error -71`) ; aucun port série n'est disponible et OpenOCD ne peut pas examiner la cible. Aucun test d'écriture/relecture n'a pu être exécuté dans cet état. Le retour de la panne après ajout simultané des fils et de la carte mémoire ne permet pas encore d'en désigner la cause.
- Victor confirme le retrait de la carte mémoire avec les fils conservés. La capture suivante ne présente toujours aucun port série et OpenOCD échoue. Le journal conserve la dernière erreur `-71`, sans nouvelle séquence de déconnexion/reconnexion dans la période examinée : ce constat ne démontre donc pas un nouvel essai d'énumération après retrait. Une vérification visuelle du câblage est nécessaire avant de poursuivre.
- **Accès USB rétabli sans déplacer de fil :** le périphérique est autorisé et actif, mais `bConfigurationValue` est vide. Une réinitialisation ciblée `USBDEVFS_RESET` sur le périphérique ST-LINK/V2.1 réussit ; Linux réattache les interfaces et `/dev/ttyACM0`, puis OpenOCD examine la cible (environ 3,23 V). L'origine de l'échec initial reste indéterminée. Le même contrôle est disponible sous Linux avec `./scripts/projet.sh usb-reset` ; script limité à une unique sonde `0483:374b`, sans changement de firmware ou réglage USB permanent.
- Victor confirme la remise de la carte mémoire. Le diagnostic SPI matériel reçoit uniquement `FF` en réponse à CMD0 ; pas de montage FAT ni d'écriture. Un diagnostic GPIO sur les mêmes contacts est ensuite compilé, téléversé et vérifié. Il lit également `FF`, avec CS lu à 1 puis 0 côté STM32. Ce résultat seul ne permet pas de conclure à une erreur de branchement.
- **Erreur de repérage corrigée :** le premier plan ICSP communiqué utilisait une autre variante de connecteur de la bibliothèque Eagle. La variante choisie pour U2 dans le schéma est `'WITH/-SILK'`, recoupée avec les nets du circuit imprimé : MISO pastille 33, SCK pastille 34, MOSI pastille 37. Dans la vue de dessus fournie, SCK est en haut milieu, MOSI en bas milieu et MISO en haut droite. Victor suivait les instructions initiales erronées ; les documents sont corrigés et il confirme le déplacement des trois contacts.
- Après correction, SPI matériel et GPIO reçoivent encore `FF` à CMD0, CS lu à 1 puis 0. La console et ST-LINK fonctionnent. Captures SD sans coordonnées personnelles : `verification/sd-physique.txt`. Aucune écriture physique n'a été réalisée. Victor confirme une carte de **8 Go** dans l'adaptateur. Un essai est demandé sur l'autre chemin prévu par le circuit : blanc D2 vers broche SD 13, jaune D4 vers SD 11, blanc D4 vers SD 12, CS sur 4 et alimentation inchangés. Aucune soudure ou modification logicielle n'est nécessaire pour ce contrôle.
- Victor confirme cet essai latéral ; SPI matériel et GPIO lisent encore `FF`. Il dispose d'un lecteur SD pour son PC, mais pas de multimètre. Une fois la carte insérée dans le lecteur du PC, Linux détecte une SDHC de 7,26 GiB et monte sa partition FAT32. Le noyau signale un précédent retrait sans démontage propre ; aucune réparation ni écriture n'est effectuée. Compte rendu : `verification/carte-sd-pc.txt`. La détection et le format sont confirmés ; cela ne valide pas le shield Seeed.
- Programme complet adapté aux câbles disponibles : SPI1 PB3/PB4/PB5, CS PA10, boutons sur D8 (PA9/PC7). Les horloges GPIOA/B/C sont activées pour les boutons ; leur initialisation ne modifie pas PB3. Les profils `release`, `debug`, `diagnostic` et `diagnostic_sd_grove` compilent sans avertissement C : `verification/compilation-montage-grove.txt`. Les commandes de compilation et de téléversement restent identiques. Un remontage des capteurs sur ce brochage est demandé pour poursuivre les tests.
- Victor confirme les modules remontés. Le programme complet `release` est téléversé et **Verified OK**. Console : mode standard, version `3WW 1.0.0 LOT=CESI-2026 BUILD=1`, valeurs par défaut car EEPROM absente ou vierge. Premier cycle physique : température **22,5 °C**, humidité **49,4 %**, pression **1010,4 hPa**, luminosité **862/1023**, catégorie forte. La date est invalide et la position GPS est `NA` dans cette capture. La carte SD reste dans le lecteur du PC. Trace publiée avec coordonnées masquées : `verification/station-montage.txt`. Le programme complet remplace le diagnostic sur la Nucleo.

Les captures série et diagnostics bruts restent locaux afin de ne pas publier une éventuelle position GPS personnelle.

### Reprise du test LED

Victor signale une LED éteinte après la tentative de démarrage en configuration et confirme la référence **Chainable RGB LED 2.0**. La notice Seeed indique 5 V, alors que le branchement direct D6 précédemment communiqué utilisait le Grove réglé sur 3V3. La procédure est corrigée : essai séparé de la LED en 5 V sur l'entrée IN, en conservant PB10/PA8 et le Grove sur 3V3. Le lecteur SD est déconnecté pendant cet essai. Aucun résultat lumineux n'est encore disponible et le mode réellement atteint pendant la tentative précédente n'a pas été confirmé par le débogueur. Au début de cette reprise, ST-LINK apparaît dans `lsusb`, mais aucun port série n'est présent.

## Validation finale encore nécessaire

- Valider les lectures RTC/BME280 avec leurs nouveaux pilotes sur le montage terminé, puis régler la date et l'heure.
- Valider la luminosité physique, le sens des boutons et les couleurs LED.
- Confirmer l'accès aux secteurs et fichiers de la SD réelle, puis montrer les archives et le remplacement en maintenance.
- Terminer l'essai du support SD séparé pour distinguer un problème de contacts mécaniques d'un problème d'alimentation ou de support, puis valider le routage SPI.
- Obtenir une position GPS valide avec vue du ciel.
- Ajouter/identifier une vraie EEPROM AT24C32, ou obtenir une clarification explicite du tuteur sur la mémoire persistante permise. La SD branchée ne résout pas automatiquement l'exigence EEPROM du PDF.
- Faire arrêter par le tuteur la règle contradictoire d'appui rouge depuis économique.
- Exécuter le scénario de démonstration et compléter la mesure de pile au pire cas.
- Victor invitera lui-même les collaborateurs de son groupe.
