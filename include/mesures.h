#ifndef MESURES_H
#define MESURES_H
#include <stdint.h>
#include <stdbool.h>
#define FENETRE 10
/* Temperature, humidite, pression : dixiemes. Lumiere : 0..1023. */
typedef struct
{
    int16_t historique[FENETRE];
    int32_t somme;
    uint8_t prochain, nombre, timeouts;
    bool valide, incoherente;
    int16_t derniere;
} Capteur;
void mesure_ajouter(Capteur *c, int16_t valeur);
void mesure_echouer(Capteur *c, bool incoherente);
int16_t mesure_moyenne(const Capteur *c);
#endif
