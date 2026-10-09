#include "parametres.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
/* Table simple : nom, domaine, champ dans la structure. */
typedef struct
{
    const char *nom;
    int32_t min, max;
    size_t champ;
} Regle;
#define REGLE(n, a, b, c) {n, a, b, offsetof(Parametres, c)}
static const Regle regles[] = {REGLE("LOG_INTERVAL", 1, 35791, log_interval),
                               REGLE("LOG_INTERVALL", 1, 35791, log_interval),
                               REGLE("FILE_MAX_SIZE", 1, 2147483647, file_max_size),
                               REGLE("TIMEOUT", 1, 4294967, timeout),
                               REGLE("LUMIN", 0, 1, lumin),
                               REGLE("LUMIN_LOW", 0, 1023, lumin_low),
                               REGLE("LUMIN_HIGH", 0, 1023, lumin_high),
                               REGLE("TEMP_AIR", 0, 1, temp_air),
                               REGLE("MIN_TEMP_AIR", -40, 85, min_temp_air),
                               REGLE("MAX_TEMP_AIR", -40, 85, max_temp_air),
                               REGLE("HYGR", 0, 1, hygr),
                               REGLE("HYGR_MINT", -40, 85, hygr_mint),
                               REGLE("HYGR_MAXT", -40, 85, hygr_maxt),
                               REGLE("PRESSURE", 0, 1, pressure),
                               REGLE("PRESSURE_MIN", 300, 1100, pressure_min),
                               REGLE("PRESSURE_MAX", 300, 1100, pressure_max)};
void parametres_defaut(Parametres *p)
{
    *p = (Parametres){10, 2048, 30, 1, 255, 768, 1, -10, 60, 1, 0, 50, 1, 850, 1080};
}
bool parametres_valides(const Parametres *p)
{
    for (size_t i = 0; i < sizeof regles / sizeof regles[0]; ++i)
    {
        int32_t v = *(const int32_t *)((const char *)p + regles[i].champ);
        if (v < regles[i].min || v > regles[i].max)
            return false;
    }
    return p->lumin_low <= p->lumin_high && p->min_temp_air <= p->max_temp_air &&
           p->hygr_mint <= p->hygr_maxt && p->pressure_min <= p->pressure_max;
}
bool parametres_modifier(Parametres *p, const char *s)
{
    const char *eq = strchr(s, '=');
    char *fin;
    if (!eq || !eq[1])
        return false;
    const char *v = eq + 1;
    if (*v == '-')
        ++v;
    if (!*v)
        return false;
    for (const char *c = v; *c; ++c)
        if (*c < '0' || *c > '9')
            return false;
    errno = 0;
    long val = strtol(eq + 1, &fin, 10);
    if (errno || *fin)
        return false;
    for (size_t i = 0; i < sizeof regles / sizeof regles[0]; ++i)
    {
        const Regle *r = &regles[i];
        if (strlen(r->nom) != (size_t)(eq - s) || strncmp(s, r->nom, eq - s))
            continue;
        if (val < r->min || val > r->max)
            return false;
        Parametres nouveau = *p;
        *(int32_t *)((char *)&nouveau + r->champ) = (int32_t)val;
        if (!parametres_valides(&nouveau))
            return false;
        *p = nouveau;
        return true;
    }
    return false;
}
bool date_valide(const RTC_DateTime *d)
{
    static const uint8_t jours[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (d->year < 2000 || d->year > 2099 || d->month < 1 || d->month > 12 || d->day < 1 ||
        d->hours > 23 || d->minutes > 59 || d->seconds > 59 || d->dayOfWeek < 1 || d->dayOfWeek > 7)
        return false;
    unsigned max = jours[d->month - 1] + (d->month == 2 && d->year % 4 == 0);
    return d->day <= max;
}
bool regler_rtc(RTC_DateTime *d, const char *s)
{
    unsigned a, b, c;
    int fin = 0;
    RTC_DateTime n = *d;
    /* On refuse signe, espaces, suffixes et valeurs trop longues avant sscanf. */
    if (!strncmp(s, "CLOCK=", 6) || !strncmp(s, "DATE=", 5))
    {
        const char *v = strchr(s, '=') + 1;
        if (strlen(v) > 14)
            return false;
        for (const char *x = v; *x; ++x)
            if ((*x < '0' || *x > '9') && *x != ':' && *x != ',')
                return false;
    }
    if (sscanf(s, "CLOCK=%u:%u:%u%n", &a, &b, &c, &fin) == 3 && s[fin] == '\0')
    {
        if (a > 23 || b > 59 || c > 59)
            return false;
        n.hours = a;
        n.minutes = b;
        n.seconds = c;
    }
    else if (sscanf(s, "DATE=%u,%u,%u%n", &a, &b, &c, &fin) == 3 && s[fin] == '\0')
    {
        if (a > 12 || b > 31 || c < 2000 || c > 2099)
            return false;
        n.month = a;
        n.day = b;
        n.year = c;
    }
    else if (!strncmp(s, "DAY=", 4))
    {
        static const char *noms[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
        unsigned i = 0;
        while (i < 7 && strcmp(s + 4, noms[i]))
            ++i;
        if (i == 7)
            return false;
        n.dayOfWeek = (i + 1) % 7 + 1; /* DS1307 : dimanche=1. */
    }
    else
        return false;
    if (!date_valide(&n))
        return false;
    *d = n;
    return true;
}
