# Worldwide Weather Watcher - Projet 3WW CESI

Programme C pour NUCLEO-L476RG, dérivé de la base imposée [ValentinMoraine/Worldwide-Weather-Watcher](https://github.com/ValentinMoraine/Worldwide-Weather-Watcher). Livrable 1 déjà rendu ; ce dépôt prépare les livrables 2, 3 et 4.

Le matériel confirmé est un capteur météo **BME280**, une **RTC Grove v2 PCF85063TP**, un GPS sur **A0**, un capteur de luminosité sur **A2**, une LED RGB sur **D6**, les boutons sur **D2**, et un support SD en SPI. Les brochages complets se trouvent dans la documentation technique.

**Validation encore incomplète :** une EEPROM externe AT24C32 est prévue pour respecter la mémoire demandée dans le PDF, mais elle n'a pas été identifiée dans le montage. Le PDF se contredit également sur le rouge maintenu cinq secondes en économique : cette version choisit retour standard et documente l'alternative. Consulter le compte rendu de validation avant la démonstration.

## Compiler et utiliser

Prérequis : PlatformIO Core, compilateur C du poste pour les tests, `mkfs.fat` pour l'image SD de test, accès USB ST-LINK.

```bash
./scripts/projet.sh test
./scripts/projet.sh compile release
./scripts/projet.sh upload release
./scripts/projet.sh monitor
```

Pour déboguer : `./scripts/projet.sh compile debug`. Pour tester les connexions : `./scripts/projet.sh diagnostic` téléverse un programme séparé de diagnostic. Pour revenir à la station, téléverser `release`.

## Documents

- [Livrable 2 : architecture](docs/LIVRABLE_2_ARCHITECTURE.md)
- [Livrable 3 : scénario de démonstration](docs/LIVRABLE_3_DEMONSTRATION.md)
- [Livrable 4 : documentation technique et câblage](docs/LIVRABLE_4_TECHNIQUE.md)
- [Livrable 4 : guide utilisateur](docs/LIVRABLE_4_UTILISATEUR.md)
- [Correspondance des exigences](docs/CONFORMITE.md)
- [Tests effectués et limites](docs/VALIDATION.md)

## Lire le code

Commencer par `src/main.c`, puis `src/station.c`. Les transitions sont isolées dans `modes.c`, les paramètres dans `parametres.c`, les moyennes dans `mesures.c`. `eeprom.c` conserve les réglages, `serie.c` réceptionne les octets et `nmea.c` valide le GPS.

L'application reste du C avec les notions des cours : structures, tableaux, pointeurs, fonctions, conditions, automate et HAL. Les détails matériels de calibration Bosch et du système de fichiers FatFs sont dans `lib/`, avec leurs notices de licence conservées. Les sources fournies par CESI gardent leur API même lorsque leur nom mentionne un composant antérieur.

Victor ajoutera lui-même les membres du groupe comme collaborateurs du dépôt.
