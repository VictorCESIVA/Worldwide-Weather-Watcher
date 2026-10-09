#include "parametres.h"
#include "mesures.h"
#include "station.h"
#include "nmea.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static void trame(const char *corps, char *sortie)
{
    unsigned crc = 0;
    for (const char *c = corps; *c; ++c)
        crc ^= (unsigned char)*c;
    sprintf(sortie, "$%s*%02X", corps, crc);
}
int main(void)
{
    Parametres p;
    parametres_defaut(&p);
    assert(parametres_valides(&p));
    assert(p.file_max_size == 2048 && p.log_interval == 10 && p.timeout == 30);
    assert(parametres_modifier(&p, "LUMIN_LOW=200"));
    assert(parametres_modifier(&p, "MIN_TEMP_AIR=-40"));
    assert(parametres_modifier(&p, "LOG_INTERVALL=1"));
    Parametres avant = p;
    const char *invalides[] = {
        "LUMIN=2",         "LUMIN=-1",         "LUMIN=1x",
        "LUMIN=",          "LUMIN=+1",         "LUMIN= 1",
        "LUMIN_LOW=1024",  "LUMIN_LOW=900",    "MIN_TEMP_AIR=-41",
        "MAX_TEMP_AIR=86", "PRESSURE_MAX=800", "PRESSURE_MIN=1101",
        "HYGR_MINT=51",    "LOG_INTERVAL=0",   "TIMEOUT=999999999999999999999",
        "VERSION=1",       "INCONNU=1"};
    for (unsigned i = 0; i < sizeof invalides / sizeof invalides[0]; ++i)
    {
        assert(!parametres_modifier(&p, invalides[i]));
        assert(!memcmp(&p, &avant, sizeof p));
    }
    RTC_DateTime d = {0, 0, 0, 1, 1, 1, 2000};
    assert(regler_rtc(&d, "DATE=2,29,2024"));
    assert(date_valide(&d));
    assert(!regler_rtc(&d, "DATE=2,29,2025"));
    assert(d.year == 2024);
    assert(regler_rtc(&d, "CLOCK=23:59:59"));
    assert(!regler_rtc(&d, "CLOCK=24:0:0"));
    assert(!regler_rtc(&d, "CLOCK=256:0:0"));
    assert(!regler_rtc(&d, "CLOCK=1:2:3x"));
    assert(regler_rtc(&d, "DAY=MON") && d.dayOfWeek == 2);
    assert(regler_rtc(&d, "DAY=SUN") && d.dayOfWeek == 1);
    assert(!regler_rtc(&d, "DAY=FUNDAY"));
    Capteur c = {0};
    int total = 0;
    for (int i = 0; i < 1000; ++i)
    {
        int valeur = i % 2 ? 32767 : -32768;
        mesure_ajouter(&c, valeur);
        if (i >= FENETRE)
            total -= ((i - FENETRE) % 2 ? 32767 : -32768);
        total += valeur;
        assert(c.somme == total);
        assert(c.nombre == (i + 1 < FENETRE ? i + 1 : FENETRE));
        assert(mesure_moyenne(&c) == total / c.nombre);
    }
    int somme = c.somme;
    mesure_echouer(&c, false);
    assert(!c.valide && c.timeouts == 1);
    mesure_echouer(&c, false);
    assert(c.timeouts == 2 && c.somme == somme);
    for (unsigned i = 0; i < 300; ++i)
        mesure_echouer(&c, false);
    assert(c.timeouts == 2);
    mesure_ajouter(&c, 200);
    assert(c.timeouts == 0 && c.valide);
    mesure_echouer(&c, true);
    assert(c.incoherente && !c.valide);
    assert(mode_apres_appui(STANDARD, STANDARD, false) == ECONOMIQUE);
    assert(mode_apres_appui(STANDARD, STANDARD, true) == MAINTENANCE);
    assert(mode_apres_appui(MAINTENANCE, ECONOMIQUE, true) == ECONOMIQUE);
    assert(mode_apres_appui(CONFIGURATION, STANDARD, true) == CONFIGURATION);
    assert(mode_apres_appui(ECONOMIQUE, STANDARD, true) == STANDARD);
    char s[200];
    GPS_Data g;
    trame("GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,", s);
    assert(nmea_gga(s, &g));
    assert(fabs(g.latitude - 48.1173) < 1e-7);
    assert(fabs(g.longitude - 11.516666667) < 1e-7);
    s[10] ^= 1;
    assert(!nmea_gga(s, &g));
    trame("GNGGA,123519,,,,,0,00,99.99,,,,,,", s);
    assert(!nmea_gga(s, &g));
    trame("GNGGA,123519,4807.038,S,01131.000,W,1,08,,,,,,,", s);
    assert(nmea_gga(s, &g) && g.latitude < 0 && g.longitude < 0);
    trame("GPGGA,123519,4860.000,N,01131.000,E,1,08,,,,,,,", s);
    assert(!nmea_gga(s, &g));
    trame("GPGGA,123519,4807.038,N,18131.000,E,1,08,,,,,,,", s);
    assert(!nmea_gga(s, &g));
    trame("GPGGA,123519,999999999999999999,N,01131.000,E,1,08,,,,,,,", s);
    assert(!nmea_gga(s, &g));
    puts("OK : commandes, dates, modes, moyennes, erreurs, GPS (ASan/UBSan).");
    return 0;
}
