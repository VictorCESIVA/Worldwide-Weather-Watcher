#ifndef SERIE_H
#define SERIE_H
#include <stdbool.h>
#include <stdint.h>
void serie_init(void);
bool serie_lire(bool gps, uint8_t *octet);
bool serie_debordement(bool gps);
#endif
