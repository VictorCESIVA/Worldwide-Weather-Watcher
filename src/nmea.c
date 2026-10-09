#include "nmea.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
static int hexa(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}
static bool coordonnee(const char *s, const char *hemi, bool latitude, double *resultat)
{
    if (!*s || strlen(hemi) != 1)
        return false;
    for (const char *c = s; *c; ++c)
        if ((*c < '0' || *c > '9') && *c != '.')
            return false;
    char *fin;
    double v = strtod(s, &fin);
    if (*fin || !isfinite(v) || v < 0 || v > (latitude ? 9000.0 : 18000.0))
        return false;
    unsigned deg = (unsigned)(v / 100);
    double min = v - deg * 100;
    if (min >= 60 || deg > (latitude ? 90u : 180u) || (deg == (latitude ? 90u : 180u) && min != 0))
        return false;
    char h = *hemi;
    if (latitude ? (h != 'N' && h != 'S') : (h != 'E' && h != 'W'))
        return false;
    *resultat = deg + min / 60;
    if (h == 'S' || h == 'W')
        *resultat = -*resultat;
    return true;
}
bool nmea_gga(const char *ligne, GPS_Data *gps)
{
    size_t len = strlen(ligne);
    if (len < 12 || len >= 96 || ligne[0] != '$')
        return false;
    const char *star = strchr(ligne, '*');
    if (!star || strlen(star) != 3 || hexa(star[1]) < 0 || hexa(star[2]) < 0)
        return false;
    unsigned crc = 0;
    for (const char *c = ligne + 1; c < star; ++c)
        crc ^= (unsigned char)*c;
    if (crc != (unsigned)(hexa(star[1]) * 16 + hexa(star[2])))
        return false;
    char copie[96];
    memcpy(copie, ligne, star - ligne);
    copie[star - ligne] = '\0';
    char *f[16] = {copie};
    unsigned n = 1;
    /* Contrairement a strtok, les champs vides sont conserves. */
    for (char *c = copie; *c && n < 16; ++c)
        if (*c == ',')
        {
            *c = '\0';
            f[n++] = c + 1;
        }
    if (n < 8 || (strcmp(f[0], "$GPGGA") && strcmp(f[0], "$GNGGA")))
        return false;
    if (strlen(f[6]) != 1 || *f[6] < '1' || *f[6] > '8')
        return false;
    if (!*f[7])
        return false;
    for (char *c = f[7]; *c; ++c)
        if (*c < '0' || *c > '9')
            return false;
    unsigned satellites = strtoul(f[7], NULL, 10);
    if (satellites > 99)
        return false;
    GPS_Data g = {0};
    if (!coordonnee(f[2], f[3], true, &g.latitude) || !coordonnee(f[4], f[5], false, &g.longitude))
        return false;
    g.valid = true;
    g.fix_quality = *f[6] - '0';
    g.satellites = satellites;
    strcpy(g.raw_sentence, ligne);
    *gps = g;
    return true;
}
