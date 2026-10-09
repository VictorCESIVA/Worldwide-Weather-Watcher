# Livrable 4 - Guide utilisateur

## Démarrer la station

Mettre sous tension sans tenir de bouton pour démarrer en standard. La LED est verte. Une ligne contient la date et l'heure, la température, l'humidité, la pression, la luminosité, sa catégorie, puis la latitude et la longitude.

Les valeurs météo et la luminosité sont les moyennes des dix dernières mesures réussies, ou des mesures disponibles au démarrage. Une mesure absente ou invalide apparaît `NA`, même si un historique existe. Le GPS affiche la position acquise pour le cycle courant.

La station enregistre normalement toutes les dix minutes. Le premier cycle commence au démarrage. Un capteur absent peut retarder la ligne de trente secondes. Un GPS sans position valide produit `NA` ; sa première localisation doit être obtenue avec une bonne vue du ciel.

## Utiliser les boutons

| Action | Résultat |
|---|---|
| Tenir rouge pendant la mise sous tension | Configuration, LED jaune |
| Standard : tenir vert cinq secondes | Économique, LED bleue |
| Standard : tenir rouge cinq secondes | Maintenance, LED orange |
| Maintenance : tenir rouge cinq secondes | Retour au mode d'origine |
| Économique : tenir rouge cinq secondes | Standard, selon la règle retenue dans cette version |
| Configuration : aucune activité série pendant trente minutes | Standard |

Relâcher un bouton avant un nouvel appui. Un appui maintenu ne déclenche qu'un seul changement.

**Ambiguïté du sujet :** le PDF décrit aussi un accès maintenance depuis économique avec le même appui rouge. Faire confirmer la règle par le tuteur ; la version livrée suit le paragraphe économique.

En économique, l'intervalle est doublé et la position GPS est acquise une fois sur deux. Le paramètre enregistré n'est pas modifié. La position est `NA` pour les cycles où le GPS est volontairement ignoré.

## Retirer ou remplacer la SD

Passer en maintenance et attendre le message **« SD fermee : retrait autorise. »** sur la console. Les mesures restent visibles, mais aucun fichier SD n'est écrit. Remplacer la carte puis revenir au mode précédent. La nouvelle carte est détectée à la prochaine écriture.

Les cartes doivent être préparées en FAT16 ou FAT32. Le programme ne formate pas les cartes. En cas de changement du câblage ou des modules, couper l'alimentation USB.

## Régler la station

Démarrer avec rouge tenu. Ouvrir le port série ST-LINK à **115200 bauds, 8 bits, sans parité, 1 bit d'arrêt**, sans contrôle de flux. Envoyer une commande par ligne, terminée par Entrée. Les fins de ligne CR, LF et CRLF sont acceptées.

| Commande | Valeur par défaut / domaine |
|---|---|
| `LOG_INTERVAL=10` | 10 minutes ; `LOG_INTERVALL` est aussi accepté car le PDF utilise les deux écritures |
| `FILE_MAX_SIZE=2048` | 2 ko, taille en octets ; `4096` donne 4 ko |
| `TIMEOUT=30` | 30 secondes |
| `RESET` | Rétablir les paramètres par défaut |
| `VERSION` | Afficher version, lot et numéro de build |
| `LUMIN=1` | Activation 0 ou 1, défaut 1 |
| `LUMIN_LOW=255` | 0 à 1023 |
| `LUMIN_HIGH=768` | 0 à 1023 |
| `TEMP_AIR=1` | Activation 0 ou 1, défaut 1 |
| `MIN_TEMP_AIR=-10` | -40 à 85 °C |
| `MAX_TEMP_AIR=60` | -40 à 85 °C |
| `HYGR=1` | Activation 0 ou 1, défaut 1 |
| `HYGR_MINT=0` | -40 à 85 °C |
| `HYGR_MAXT=50` | -40 à 85 °C |
| `PRESSURE=1` | Activation 0 ou 1, défaut 1 |
| `PRESSURE_MIN=850` | 300 à 1100 hPa |
| `PRESSURE_MAX=1080` | 300 à 1100 hPa |
| `CLOCK=14:30:00` | Heure 0-23, minute et seconde 0-59 |
| `DATE=10,9,2026` | Mois, jour, année 2000-2099 ; date réelle obligatoire |
| `DAY=FRI` | MON, TUE, WED, THU, FRI, SAT ou SUN |

Les minima doivent rester inférieurs ou égaux aux maxima. Pour déplacer une plage entière, modifier d'abord le seuil permettant de garder une plage cohérente.

`HYGR_MINT` et `HYGR_MAXT` concernent **la température de l'air à laquelle l'humidité est prise en compte**. Ils ne sont pas des pourcentages d'humidité.

Une modification réussie affiche `OK EEPROM`. Si l'EEPROM est absente ou inaccessible, la modification est refusée et les paramètres précédents sont conservés. La SD conserve les mesures ; l'EEPROM demandée pour les réglages est un autre composant. Les commandes d'horloge écrivent directement dans la RTC.

Les bornes techniques de cette version sont : intervalle de 1 à 35791 minutes, timeout de 1 à 4294967 secondes et taille de fichier de 1 à 2147483647 octets. Choisir une taille supérieure à une ligne complète, par exemple 2048 ou 4096 octets. Ces bornes protègent les calculs entiers ; le sujet ne fixe pas de maxima pour ces trois paramètres.

## Comprendre la LED

| Signal | Signification |
|---|---|
| Verte continue | Standard |
| Jaune continue | Configuration |
| Bleue continue | Économique |
| Orange continue | Maintenance |
| Rouge/bleu, parts égales, cycle d'une seconde | Défaut RTC |
| Rouge/jaune, parts égales, cycle d'une seconde | Défaut GPS |
| Rouge/vert, parts égales, cycle d'une seconde | Capteur inaccessible |
| Rouge/vert, vert deux fois plus long | Donnée incohérente |
| Rouge/blanc, parts égales, cycle d'une seconde | SD pleine |
| Rouge/blanc, blanc deux fois plus long | Défaut d'accès ou écriture SD |

Un défaut d'acquisition est signalé après deux cycles en timeout consécutifs. Une lecture réussie remet son compteur à zéro. Les données incohérentes sont signalées immédiatement. Si plusieurs défauts existent, la priorité est RTC, GPS, capteurs, incohérence, puis SD. La console et les valeurs `NA` aident à trouver les autres problèmes.

## Lire les fichiers

Pour le 9 octobre 2026, le fichier courant est `261009_0.LOG`. La station conserve des lignes entières. Avant de dépasser la limite, elle copie le fichier courant dans la première révision libre (`261009_1.LOG`, puis `_2`, etc.), ferme la copie, puis repart au début du fichier `_0`. Si la limite est atteinte exactement, l'archivage est immédiat.

Les champs sont séparés par `;`. Les décimales utilisent un point. Il est possible d'importer ces fichiers comme texte délimité dans un tableur. Le fichier ne comporte pas de ligne d'en-tête ; l'ordre des huit colonnes est :

`date ; temperature_C ; humidite_pct ; pression_hPa ; lumi_0_1023 ; niveau ; latitude ; longitude`

En cas de RTC invalide, la ligne est visible sur la console avec une date `NA`, mais aucune écriture SD n'est effectuée : il faut d'abord régler ou réparer l'horloge.
