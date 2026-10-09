# Worldwide Weather Watcher - Projet 3WW CESI

Programme C pour NUCLEO-L476RG, dérivé de la base imposée [ValentinMoraine/Worldwide-Weather-Watcher](https://github.com/ValentinMoraine/Worldwide-Weather-Watcher). Livrable 1 déjà rendu ; ce dépôt prépare les livrables 2, 3 et 4.

Le matériel identifié est un capteur météo **BME280**, une **RTC Grove v2 PCF85063TP**, un GPS raccordé sur **A0**, un Grove Base Shield **v2.1** et un Seeed SD Card Shield **v4.4**. Le programme prévoit la luminosité sur **A2**, les signaux LED RGB correspondant à **D6/D7** et les boutons sur **D8**. La **Chainable RGB LED 2.0 demande une alimentation séparée en 5 V** lorsque le Grove reste sur 3V3 : voir [l'essai LED](docs/TEST_LED.md). Les câbles Grove **D2/D4** servent au SPI du lecteur SD placé à côté. Les brochages complets et les validations restantes se trouvent dans la documentation technique.

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

Le test isolé `./scripts/projet.sh diagnostic-sd` utilise un câblage différent, réservé au lecteur SD posé à côté de la carte. Suivre [sa procédure de câblage](docs/TEST_SD_GROVE.md) avant de l'utiliser. Ce diagnostic ne constitue pas la station complète.

Sous Linux, si ST-LINK est détecté mais que la configuration USB échoue, `./scripts/projet.sh usb-reset` peut relancer son énumération. Cette récupération a rétabli la connexion sur le poste sans déplacer de fil ; elle ne modifie pas le firmware.

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
