# Essai de la Chainable RGB LED 2.0

Le module utilise un P9813S14 et demande **5 V** selon la [notice Seeed](https://wiki.seeedstudio.com/Grove-Chainable_RGB_LED/). Le Base Shield reste sur **3V3** pour le capteur de luminosité. Cet essai isole la LED ; il ne valide pas encore la station complète avec le lecteur SD.

1. Débrancher l'USB ST-LINK.
2. Déconnecter le lecteur SD : retirer son alimentation rouge/noir de CN7 et les prises Grove D2/D4. Conserver la carte mémoire dans le lecteur du PC.
3. Retirer le câble qui reliait D6 à la LED.
4. Brancher la prise blanche du quatrième câble Grove vers Dupont femelles sur **IN**, repéré CI/DI, de la LED. Laisser OUT libre.
5. Raccorder les quatre extrémités selon le tableau, puis rebrancher l'USB ST-LINK.

| Fil | Contact Nucleo | Fonction |
|---|---|---|
| Rouge | CN7-18, +5V | Alimentation LED |
| Noir | CN7-20, GND | Masse commune |
| Jaune | CN10-25, PB10 | Horloge, correspondant à D6 |
| Blanc | CN10-23, PA8 | Données, correspondant à D7 |

Ces contacts sont donnés dans le [manuel ST UM1724, tableau 32](https://www.st.com/resource/en/user_manual/dm00105823.pdf). Avec le connecteur USB ST-LINK en haut, CN7 est la longue double rangée gauche et CN10 celle de droite. Compter depuis le haut dans la colonne **proche du centre** : CN7 contact 9 = broche 18, contact 10 = broche 20 ; CN10 contact 12 = broche 23, contact 13 = broche 25. Ne pas confondre le numéro de broche avec le numéro de rangée.

Les autres modules peuvent rester sur le Grove : BME280 et RTC sur I²C, GPS sur A0, luminosité sur A2, boutons sur D8. Le port UART reste libre. Aucun changement de firmware n'est nécessaire : PB10 et PA8 sont déjà les sorties du pilote RGB.

Pour une observation stable, tenir le bouton rouge pendant la mise sous tension ou le reset, puis le relâcher après une seconde. Si le mode configuration est atteint, la couleur prévue est jaune. Vérifier aussi le mode par la console ou le débogueur : une LED éteinte seule ne prouve pas que le bouton ou le mode a échoué. Les couleurs physiques et le partage de l'alimentation 5V avec le lecteur SD restent à confirmer.
