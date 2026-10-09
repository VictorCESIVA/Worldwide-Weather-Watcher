#ifndef STATION_H
#define STATION_H
#include "parametres.h"
#include "mesures.h"
typedef enum
{
    STANDARD,
    CONFIGURATION,
    MAINTENANCE,
    ECONOMIQUE
} Mode;
typedef enum
{
    ERREUR_AUCUNE,
    ERREUR_RTC,
    ERREUR_GPS,
    ERREUR_CAPTEUR,
    ERREUR_INCOHERENTE,
    ERREUR_SD_PLEINE,
    ERREUR_SD
} Erreur;
/* Le PDF se contredit. 1 suit le paragraphe economique (rouge -> standard).
 * 0 suit le paragraphe maintenance (rouge -> maintenance depuis economique).
 */
#ifndef ROUGE_ECO_VERS_STANDARD
#define ROUGE_ECO_VERS_STANDARD 1
#endif
void station_init(void);
void station_boucle(void);
Mode mode_apres_appui(Mode actuel, Mode precedent, bool rouge);
#endif
