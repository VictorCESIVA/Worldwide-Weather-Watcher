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
rangée du haut      non utilisé  SCK          MISO
rangée du bas       non utilisé  MOSI         non utilisé
```

La vue par-dessous inverse gauche et droite. Seuls ces trois contacts ICSP sont utilisés. Le repérage vient de la variante **effectivement choisie** pour U2 dans le [fichier Eagle Seeed v4.3](https://files.seeedstudio.com/wiki/SD_Card_shield_V4.0/res/SD_Card_Shield_v4.3_eagle_file.zip) : `'WITH/-SILK'`, pastilles 33 MISO, 34 SCK, 37 MOSI. Le premier plan communiqué utilisait une autre variante de cette bibliothèque et était erroné ; ce dessin le corrige. La continuité des sélecteurs DI/DO/CLK sur l'exemplaire v4.4 reste à confirmer. Si le lecteur ne répond pas, vérifier cette continuité avant de conclure à une panne du logiciel.

Les signaux PB3/PB4/PB5 possèdent la fonction SPI1 en AF5 dans la [fiche technique STM32L476](https://www.st.com/resource/en/datasheet/stm32l476re.pdf). Le programme emploie ce SPI matériel. Les boutons du programme complet sont sur D8 ; les autres modules restent débranchés pendant ce diagnostic isolé.

### Autre chemin de routage du lecteur

Le fichier constructeur relie la position A des sélecteurs à ICSP, et la position B aux broches Arduino 11/12/13. Sans souder ni changer le programme, un contrôle de ce second chemin consiste à débrancher l'USB, déplacer le blanc D2 vers la broche latérale SD **13**, le jaune D4 vers **11** et le blanc D4 vers **12**, puis reconnecter l'USB. Le jaune D2 reste sur **4** et l'alimentation reste identique. La réussite sur ce chemin identifierait le routage utilisable ; l'absence de réponse sur les deux chemins exige un contrôle électrique du lecteur et/ou de la carte mémoire. Il ne faut pas déplacer les sélecteurs par soudure sans avoir identifié la révision réelle et mesuré les continuités.

## Programme et résultat attendu

Le diagnostic se compile avec `pio run -e diagnostic_sd_grove`. Le téléverser avec `./scripts/projet.sh diagnostic-sd`, puis insérer une carte FAT16/FAT32 avant le redémarrage et contrôler le résultat. Aucune commande de ce programme ne formate la carte.

Le programme tente l'initialisation, monte FatFs, choisit le premier nom libre entre `3WWT000.LOG` et `3WWT999.LOG`, écrit `3WW test SD` suivi de CR/LF (13 octets), synchronise, ferme et relit le fichier. `FA_CREATE_NEW` protège les fichiers déjà présents. La réussite exige `init=1`, `montee=1`, `ecriture=1`, `relecture=1`, `test=1`. Un fichier partiel peut rester en cas d'échec ; il n'est pas effacé automatiquement.

Si la console ne fonctionne pas, la sonde peut lire `diag_sd_init`, `diag_sd_statut`, `diag_sd_montage`, `diag_sd_ecriture` et `diag_sd_lecture`. Après un échec d'initialisation, le diagnostic compare aussi CMD0 en pilotant directement les mêmes GPIO, avec une horloge plafonnée à 200 kHz par le compteur de cycles du Cortex-M4. `diag_sd_cmd0_gpio` expose le résultat ; ce test n'est pas compilé dans la station. Le bouton RESET relance un essai et peut créer un nouveau fichier. Pour revenir au programme complet, téléverser `release`, conserver les adaptateurs D2/D4 et brancher les boutons sur D8.

## Récupération de la connexion USB sous Linux

Si Linux détecte ST-LINK mais laisse sa configuration USB vide (`can't set config #1, error -71`), une réinitialisation ciblée peut relancer son énumération : `./scripts/projet.sh usb-reset`. Ce script vise uniquement un périphérique ST-LINK/V2.1 `0483:374b`, refuse plusieurs sondes et emploie `USBDEVFS_RESET`. Il ne modifie pas le firmware et ne change aucun réglage permanent du PC. Les permissions USB doivent déjà autoriser l'utilisateur, comme pour OpenOCD.

Cette opération a rétabli `/dev/ttyACM0` et l'accès OpenOCD sur le montage, sans déplacer de fil. Elle ne démontre pas l'origine exacte de l'échec initial. La [documentation Linux](https://cdn.kernel.org/doc/html/latest/driver-api/usb/error-codes.html) traite `EPROTO` comme une erreur de protocole USB ; elle ne suffit pas à attribuer une faute au câblage du projet.

État au 09/10/2026 : compilation et téléversement vérifiés. L'accès USB et la console sont rétablis après réinitialisation ciblée de ST-LINK, sans déplacement de fil. Le repérage ICSP initial était erroné ; il est corrigé dans ce document. Les deux chemins, ICSP puis 11/12/13, ont été essayés avec une carte de 8 Go : SPI matériel et GPIO reçoivent `FF` à CMD0. Le lecteur du PC détecte cette carte et monte son volume FAT32. Le routage DI/DO/CLK et l'alimentation du shield restent à contrôler électriquement. Aucune écriture/relecture depuis la Nucleo n'a encore été validée.
