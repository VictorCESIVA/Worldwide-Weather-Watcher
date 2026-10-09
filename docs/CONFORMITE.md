# Correspondance entre exigences et réalisation

Les pages se réfèrent au PDF CESI « Mode de fonctionnement », cinq pages, version fournie dans les ressources du projet. Les tests réels sont séparés des tests simulés dans `VALIDATION.md`.

| Exigence | Source | Réalisation | Vérification |
|---|---|---|---|
| Partir de la base fournie | Page projet | Historique du dépôt source conservé | Git |
| Collaborateurs du groupe | Page projet | Victor les invitera lui-même | Action utilisateur restante |
| Architecture sans programme final | Livrable 2 | `LIVRABLE_2_ARCHITECTURE.md` | Document |
| Code complet, script, montage, scénario | Livrable 3 | Sources, scripts, tableau de câblage, scénario | Compilation + démonstration physique |
| Documentation technique et utilisateur | Livrable 4 | Deux documents dédiés | Lecture |
| Standard au démarrage | PDF p.1 | `station_init` | Boucle simulée, carte |
| Configuration au démarrage rouge | PDF p.1 | Lecture bouton initial, acquisitions désactivées | Boucle simulée + démarrage rouge confirmé physiquement (BUILD=2) |
| Retour après 30 minutes sans activité | PDF p.1 | `activite`, différence en millisecondes | Horloge simulée sur vraie boucle |
| Maintenance, retour au précédent | PDF p.1 et p.5 | Mode mémorisé, démontage SD | Boucle simulée + physique |
| Économique : intervalle x2, GPS un sur deux | PDF p.5 | Intervalle temporaire, compteur de cycles | Boucle simulée |
| Rouge depuis économique | PDF p.1, contradiction interne | Constante explicite, choix par défaut retour standard | Décision tuteur restante |
| Couleurs et défauts | PDF p.1-2 | `led`, cycle d'une seconde | Logique + couleurs physiques à vérifier |
| Intervalle 10 min, timeout 30 s | PDF p.2 | Paramètres par défaut | Test C |
| Ligne horodatée, NA | PDF p.2 | Cycle, format huit champs | Boucle simulée + console réelle |
| Limite 2 ko, fichiers AAMMJJ_0.LOG | PDF p.2 | FatFs, limite 2048, archives numérotées | Vrai FatFs sur image FAT16 |
| Sauvegarde en EEPROM | PDF p.2 | AT24C32, deux copies et CRC | Simulation d'EEPROM ; composant absent à résoudre |
| Paramètres et RESET/VERSION | PDF p.2-4 | Validation et sauvegarde atomique au niveau applicatif | Tests domaines et refus |
| TIMEOUT puis défaut après deux mesures | PDF p.3 | Deux timeouts consécutifs, succès remet à zéro | Test compteur + vraie boucle |
| LUMIN et catégories | PDF p.3 | ADC 12 bits normalisé 10 bits, seuils inclusifs dans catégorie moyenne | Code + montage |
| Température -40 à 85, seuils -10/60 | PDF p.3-4 | Types et validation avant conversion | Tests C |
| Hygrométrie filtrée par température | PDF p.4 | `HYGR_MINT`, `HYGR_MAXT` | Boucle |
| Pression 300-1100, seuils 850/1080 | PDF p.4 | Valeurs converties et bornées | Tests C |
| CLOCK, DATE, DAY | PDF p.5 | Date réelle, BCD, jour explicite ; dernier réglage gardé si relecture temporairement indisponible | Tests calendaires, vraie boucle en erreur de relecture, RTC physique après reset |
| C modulaire, build et debug/release | Prosit 3 | PlatformIO, Bash, numéro de build | Compilation deux profils |
| Structures et optimisation des mesures | Prosit 4 | `Capteur`, passage par adresse, somme conservée | 1000 insertions et bornes int16 |
| Gestion mémoire et pile | Prosit 5 | État statique, sections ELF, fichiers `.su` | Rapport mémoire ; pic physique à mesurer |

## Interprétations rendues explicites

- Le PDF écrit à la fois `LOG_INTERVAL` et `LOG_INTERVALL` : les deux noms sont acceptés.
- Le défaut de limite est 2048 octets. `FILE_MAX_SIZE=4096` est un exemple de commande, pas une autre valeur par défaut.
- Les deux timeouts sont considérés consécutifs ; un succès rétablit le compteur.
- Les moyennes utilisent dix mesures **réussies**, conformément au traitement optimisé étudié. Une erreur courante ne réutilise pas une ancienne valeur à la place de `NA`.
- Une ligne est archivée avant d'être coupée par une limite de fichier ; la révision 0 reste le fichier d'écriture.
- Sans date RTC valide, l'écriture est empêchée pour ne pas inventer un nom de fichier daté.
- Aucun substitut à l'EEPROM et aucune règle contradictoire de bouton ne sont présentés comme une conformité définitive. Ces deux points doivent être résolus avant validation finale.
