#include "mesures.h"
void mesure_ajouter(Capteur *c, int16_t v)
{
    c->valide = true;
    c->incoherente = false;
    c->timeouts = 0;
    c->derniere = v;
    if (c->nombre == FENETRE)
        c->somme -= c->historique[c->prochain];
    else
        ++c->nombre;
    c->historique[c->prochain] = v;
    c->somme += v;
    c->prochain = (c->prochain + 1) % FENETRE;
}
void mesure_echouer(Capteur *c, bool incoherente)
{
    c->valide = false;
    c->incoherente = incoherente;
    if (!incoherente && c->timeouts < 2)
        ++c->timeouts;
    if (incoherente)
        c->timeouts = 0;
}
int16_t mesure_moyenne(const Capteur *c)
{
    return c->nombre ? c->somme / c->nombre : 0;
}
