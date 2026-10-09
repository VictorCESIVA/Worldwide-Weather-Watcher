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

`pio run -e release -e debug` : deux profils compilés avec succès, sans avertissement dans la dernière trace.

| Profil | RAM statique annoncée | Flash annoncée |
|---|---:|---:|
| Release | 5288 octets / 98304 | 61260 octets / 1048576 |
| Debug | 5332 octets / 98304 | 92104 octets / 1048576 |

Ces chiffres correspondent aux dernières sources compilées lors de la rédaction. La RAM annoncée ne mesure pas le pic de pile ni les allocations internes des bibliothèques C. L'application n'appelle pas `malloc`, mais les bibliothèques C liées, notamment la conversion de texte en nombre, peuvent utiliser le tas.

Traces : `verification/compilation.txt`, `verification/memoire-release.txt`. Les cadres estimés par GCC sont filtrés sur les fonctions encore présentes dans l'ELF. Le pic de pile avec les appels de bibliothèque et les interruptions reste à mesurer physiquement.

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
- Victor indique ne pas avoir de fils Dupont. L'essai séparé et le raccordement ICSP sont donc actuellement impossibles. Il précise que le connecteur ICSP SD s'emboîte sur son Grove. Vérification du schéma officiel Grove Base Shield v2 : ce modèle ne relie pas les contacts ICSP MOSI/MISO/SCK aux nets D11/D12/D13. La référence du Grove de Victor reste à confirmer ; aucune liaison SPI fonctionnelle n'a été démontrée.
- Une vérification des capteurs sans support SD a été proposée : BME280 et RTC sur I²C, GPS sur A0, UART Grove libre. Ce remontage n'est pas encore confirmé.

Les captures série et diagnostics bruts restent locaux afin de ne pas publier une éventuelle position GPS personnelle.

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
