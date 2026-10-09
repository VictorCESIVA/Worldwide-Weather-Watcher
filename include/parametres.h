#ifndef PARAMETRES_H
#define PARAMETRES_H
#include <stdbool.h>
#include <stdint.h>
#include "cesi_types.h"
typedef struct
{
    int32_t log_interval, file_max_size, timeout;
    int32_t lumin, lumin_low, lumin_high;
    int32_t temp_air, min_temp_air, max_temp_air;
    int32_t hygr, hygr_mint, hygr_maxt;
    int32_t pressure, pressure_min, pressure_max;
} Parametres;
void parametres_defaut(Parametres *p);
bool parametres_valides(const Parametres *p);
bool parametres_modifier(Parametres *p, const char *commande);
bool date_valide(const RTC_DateTime *d);
bool regler_rtc(RTC_DateTime *d, const char *commande);
bool parametres_charger(Parametres *p);
bool parametres_sauver(const Parametres *p);
#endif
