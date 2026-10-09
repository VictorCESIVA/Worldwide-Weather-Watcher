# Livrable 3 - Maquette et scénario de démonstration

La démonstration concerne la carte physique. Les tests logiciels automatisés la préparent mais ne la remplacent pas.

## Conditions préalables

Montage décrit dans la documentation technique, SD FAT16/FAT32 avec espace libre, RTC réglée, GPS avec position valide si l'on veut démontrer la géolocalisation, EEPROM AT24C32 pour démontrer la sauvegarde persistante des réglages. Le poste dispose de PlatformIO et de l'accès ST-LINK. L'absence d'EEPROM doit être résolue avant de présenter la configuration comme conforme.

1. Montrer la structure du dépôt issu de la base CESI et les documents des livrables.
2. Exécuter `./scripts/projet.sh test`, puis `./scripts/projet.sh compile release`.
3. Montrer version et numéro de build ; compiler en debug pour expliquer la différence d'optimisation.
4. Exécuter `./scripts/projet.sh upload release` et montrer la vérification ST-LINK.

## Démonstration fonctionnelle

| Étape | Manipulation | Résultat attendu / preuve |
|---|---|---|
| Standard | Démarrer sans bouton | LED verte, mode standard annoncé |
| Mesure | Attendre le premier cycle | Une ligne, huit champs, valeurs météo réelles |
| Luminosité | Masquer puis éclairer le capteur entre cycles | Valeur et catégorie changent ; moyenne explique l'inertie |
| Configuration | Tenir rouge pendant reset | LED jaune, absence d'acquisition |
| Commandes | `VERSION`, `LOG_INTERVAL=1`, `TIMEOUT=5`, `FILE_MAX_SIZE=256` | Réponses explicites ; sauvegarde EEPROM vérifiée |
| Validation | `LUMIN=2`, `MIN_TEMP_AIR=-41`, `PRESSURE_MAX=800` | Refus, paramètres précédents conservés |
| Horloge | Régler `DATE`, `CLOCK`, `DAY` | Date réelle et jour relus |
| Persistance | Couper puis remettre l'alimentation | Réglages identiques depuis EEPROM |
| Maintenance | Depuis standard, tenir rouge cinq secondes | LED orange, « retrait autorise », données série, aucune écriture SD |
| Remplacement SD | En maintenance, remplacer la SD | Remplacement sans fichier ouvert ; retour rouge cinq secondes |
| Économique | Depuis standard, tenir vert cinq secondes | LED bleue, intervalle doublé, GPS une fois sur deux |
| Retour | En économique, tenir rouge cinq secondes | Standard selon la règle documentée ; à valider avec le tuteur |
| Archives | Avec petite limite, attendre plusieurs cycles | `_0.LOG` actif ; `_1.LOG`, `_2.LOG`, etc. contiennent les anciennes lignes |
| Accès SD | Utiliser une carte indisponible ou un montage SD débranché hors tension | Erreur d'accès identifiée |
| SD pleine | Utiliser une carte de test pleine | Rouge/blanc à parts égales |
| Timeout | Hors tension, débrancher un capteur, puis redémarrer | `NA` après timeout ; défaut d'accès après deux cycles |
| Incohérence | Régler un seuil compatible avec le domaine mais excluant la mesure réelle | `NA`, rouge/vert, vert deux fois plus long |
| Mémoire | `./scripts/projet.sh memoire release` | Sections ELF, données statiques et cadres GCC expliqués |

Les changements électriques se font hors tension. Le retrait de la carte SD seule se fait en maintenance, après le message d'autorisation. Utiliser une carte de test pour les défauts, conserver les données importantes.

## Démonstration rapide

Le programme conserve les valeurs normales du sujet. Pour raccourcir la présentation, modifier les paramètres depuis la console en configuration : `LOG_INTERVAL=1`, `TIMEOUT=5`, `FILE_MAX_SIZE=256`. Cette manipulation suppose une EEPROM opérationnelle et doit être montrée comme une configuration utilisateur, pas comme un changement caché des valeurs par défaut.

En standard, un cycle arrive chaque minute ; en économique, toutes les deux minutes. La maintenance produit des lignes rapprochées. Restaurer ensuite `RESET` pour retrouver 10 minutes, 30 secondes et 2048 octets.

## Questions à préparer

- Pourquoi une structure par capteur et un tampon circulaire ?
- Pourquoi un entier de 16 bits convient aux mesures météo mais pas aux coordonnées GPS ?
- Pourquoi une somme conservée réduit-elle la complexité à O(1) ?
- Quelle différence entre donnée absente et donnée incohérente ?
- Pourquoi le premier historique ne doit-il pas être rempli de zéros artificiels ?
- Comment la copie EEPROM précédente survit-elle à une coupure pendant la nouvelle sauvegarde ?
- Pourquoi fermer les fichiers et démonter la SD avant son retrait ?
- Pourquoi la somme des tailles `.su` ne donne-t-elle pas le pic réel de pile ?
- Quel comportement a été retenu pour la contradiction du PDF sur le bouton rouge ?

Le compte rendu factuel des vérifications effectuées est dans [VALIDATION.md](VALIDATION.md).
