# Diagnostic isolé du lecteur SD avec les câbles disponibles

Ce test concerne la NUCLEO-L476RG, le Grove Base Shield v2.1 et le Seeed SD Card Shield v4.4. Il sert à isoler le défaut USB observé avec le lecteur empilé, puis à vérifier une écriture et sa relecture. Il ne remplace pas le programme de la station.

## Préparation et alimentation seule

Débrancher l'USB pendant chaque modification. Garder uniquement Nucleo → Grove, sans modules branchés sur les ports Grove. Laisser le sélecteur de tension Grove sur **3V3**. Poser le lecteur SD à côté, carte mémoire retirée, sur une surface isolante.

Un adaptateur comporte une prise blanche Grove et quatre prises Dupont femelles. Sur les adaptateurs photographiés : rouge = alimentation, noir = masse, jaune = premier signal, blanc = second signal.

Brancher la prise blanche d'un adaptateur dans le port **I²C du lecteur SD**, dont le connecteur est marqué 5V. Relier uniquement :

| Bout du câble | Contact mâle de la Nucleo |
|---|---|
| Rouge | CN7 broche 18, +5V |
| Noir | CN7 broche 20, GND |
| Jaune et blanc | Non raccordés |

Avec l'USB ST-LINK en haut, CN7 est la longue double rangée à gauche. Dans la colonne la plus proche du centre, compter depuis le haut : neuvième contact = broche 18, dixième = broche 20. Le [manuel ST UM1724, tableau 32](https://www.st.com/resource/en/user_manual/dm00105823.pdf) confirme les affectations. Ne pas utiliser VIN, E5V ou U5V à la place du contact +5V.

Rebrancher l'USB et contrôler l'énumération ST-LINK et le port série. Si la connexion échoue encore, retirer l'alimentation du lecteur et résoudre cette panne avant de connecter SPI. Le port I²C du lecteur sert ici seulement à son alimentation ; les fils SDA/SCL restent libres.

## Signaux, après validation de l'alimentation

Débrancher l'USB. Ajouter deux adaptateurs sur le **Base Shield Grove** :

| Port Grove | Bout du câble | Contact mâle du lecteur SD |
|---|---|---|
| D2 | Jaune (D2 / PA10) | Broche Arduino marquée 4 : CS |
| D2 | Blanc (D3 / PB3) | ICSP SCK |
| D4 | Jaune (D4 / PB5) | ICSP MOSI |
| D4 | Blanc (D5 / PB4) | ICSP MISO |
| D2 et D4 | Rouge et noir | Non raccordés |

Les prises femelles se raccordent aux véritables contacts mâles accessibles du lecteur. Ne pas forcer sur une soudure ou introduire de fil dans le logement de la carte mémoire.

Repérage **vu de dessus, logement SD en haut, connecteur ICSP en bas**, comme sur la première photo fournie :

```text
                    gauche       milieu       droite
rangée du haut      non utilisé  MOSI         non utilisé
rangée du bas       non utilisé  SCK          MISO
```

La vue par-dessous inverse gauche et droite. Seuls ces trois contacts ICSP sont utilisés. Le repérage vient des coordonnées du [fichier Eagle Seeed v4.3](https://files.seeedstudio.com/wiki/SD_Card_shield_V4.0/res/SD_Card_Shield_v4.3_eagle_file.zip) et des photos ; la continuité des sélecteurs DI/DO/CLK sur l'exemplaire v4.4 reste à confirmer. Si le lecteur ne répond pas, vérifier cette continuité avant de conclure à une panne du logiciel.

Les signaux PB3/PB4/PB5 possèdent la fonction SPI1 en AF5 dans la [fiche technique STM32L476](https://www.st.com/resource/en/datasheet/stm32l476re.pdf). Le programme emploie ce SPI matériel. Les boutons normalement sur D2 doivent rester débranchés pendant ce test.

## Programme et résultat attendu

Le diagnostic se compile avec `pio run -e diagnostic_sd_grove`. Le téléverser avec `./scripts/projet.sh diagnostic-sd`, puis insérer une carte FAT16/FAT32 avant le redémarrage et contrôler le résultat. Aucune commande de ce programme ne formate la carte.

Le programme tente l'initialisation, monte FatFs, choisit le premier nom libre entre `3WWT000.LOG` et `3WWT999.LOG`, écrit `3WW test SD` suivi de CR/LF (13 octets), synchronise, ferme et relit le fichier. `FA_CREATE_NEW` protège les fichiers déjà présents. La réussite exige `init=1`, `montee=1`, `ecriture=1`, `relecture=1`, `test=1`. Un fichier partiel peut rester en cas d'échec ; il n'est pas effacé automatiquement.

Si la console ne fonctionne pas, la sonde peut lire `diag_sd_init`, `diag_sd_statut`, `diag_sd_montage`, `diag_sd_ecriture` et `diag_sd_lecture`. Le bouton RESET relance un essai et peut créer un nouveau fichier. Après le diagnostic, le programme `release` exige à nouveau son propre brochage décrit dans la documentation technique. Ce câblage D2/D4 n'est pas encore un profil de station complète.

État au 09/10/2026 : compilation et téléversement vérifiés ; exécution confirmée par GDB sans lecteur raccordé. L'alimentation séparée seule, sans carte mémoire, laisse le port USB présent et la cible accessible par ST-LINK. Après confirmation des quatre signaux raccordés et de la carte mémoire insérée, l'erreur de configuration USB `-71` réapparaît. Le contrôle sans carte mémoire avec ces mêmes fils est demandé ; aucune écriture/relecture physique n'a encore été validée.
