/* Execute la vraie boucle avec une horloge et des peripheriques controles. */
#include "station.h"
#include "serie.h"
#include "sd_logger.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
UART_HandleTypeDef huart2, GPS_UART_HANDLE;
static uint32_t temps;
static bool r, v, eeprom_ok = true, rtc_present = true, env_present = true;
static bool rtc_lisible = true;
static RTC_DateTime rtc_memoire = {0, 0, 12, 6, 1, 1, 2000};
static Parametres sauvegarde;
static char entree[256], trame[128], derniere_ligne[256];
static unsigned ei, gi, ecritures, fermetures, lectures;
static uint8_t led_r, led_g, led_b;
uint32_t HAL_GetTick(void)
{
    return temps;
}
void GroveButton_Init(void)
{
}
void GroveRGB_Init(void)
{
}
void GroveLight_Init(void)
{
}
void GroveGPS_Init(void)
{
}
bool GroveButton1_IsPressed(void)
{
    return r;
}
bool GroveButton2_IsPressed(void)
{
    return v;
}
void GroveRGB_SetColor(uint8_t a, uint8_t b, uint8_t c)
{
    led_r = a;
    led_g = b;
    led_b = c;
}
uint16_t GroveLight_ReadRaw(void)
{
    ++lectures;
    return 2048;
}
bool GroveBME680_ReadEnvironment(Env_Data *d)
{
    ++lectures;
    if (!env_present)
        return false;
    *d = (Env_Data){20, 50, 1013, 0};
    return true;
}
bool GroveRTC_GetDateTime(RTC_DateTime *d)
{
    if (!rtc_present || !rtc_lisible)
        return false;
    *d = rtc_memoire;
    return true;
}
bool GroveRTC_SetDateTime(const RTC_DateTime *d)
{
    if (!rtc_present || !date_valide(d))
        return false;
    rtc_memoire = *d;
    return true;
}
bool parametres_charger(Parametres *p)
{
    parametres_defaut(p);
    p->log_interval = 1;
    p->timeout = 1;
    sauvegarde = *p;
    return true;
}
bool parametres_sauver(const Parametres *p)
{
    if (!eeprom_ok)
        return false;
    sauvegarde = *p;
    return true;
}
bool SDLogger_Mount(void)
{
    return true;
}
void SDLogger_Unmount(void)
{
    ++fermetures;
}
bool SDLogger_Append(const char *nom, const char *ligne, uint32_t limite)
{
    assert(!strcmp(nom, "261009_0.LOG"));
    assert(limite == 2048);
    strcpy(derniere_ligne, ligne);
    ++ecritures;
    return true;
}
SDLoggerResult SDLogger_GetResult(void)
{
    return SDLOGGER_OK;
}
void serie_init(void)
{
}
bool serie_debordement(bool gps)
{
    (void)gps;
    return false;
}
bool serie_lire(bool gps, uint8_t *octet)
{
    char *s = gps ? trame : entree;
    unsigned *i = gps ? &gi : &ei;
    if (!s[*i])
        return false;
    *octet = s[(*i)++];
    return true;
}
static void envoyer_gps(void)
{
    const char *s = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
    unsigned crc = 0;
    for (const char *c = s; *c; ++c)
        crc ^= (unsigned char)*c;
    sprintf(trame, "$%s*%02X\n", s, crc);
    gi = 0;
}
static void avancer(unsigned ms)
{
    for (unsigned i = 0; i < ms; i += 10)
    {
        temps += 10;
        station_boucle();
    }
}
static void bouton(bool rouge)
{
    if (rouge)
        r = true;
    else
        v = true;
    avancer(5100);
    r = v = false;
    avancer(100);
}
int main(void)
{
    r = true;
    station_init();
    unsigned avant = lectures;
    avancer(1000);
    assert(lectures == avant);
    assert(led_r == 255 && led_g == 255 && led_b == 0);
    /* Une relecture indisponible entre DATE, DAY et CLOCK ne doit pas
     * effacer la date que le composant vient d'accepter.
     */
    strcpy(entree, "DATE=10,9,2026\n");
    ei = 0;
    avancer(100);
    assert(rtc_memoire.year == 2026 && rtc_memoire.month == 10 && rtc_memoire.day == 9);
    rtc_lisible = false;
    strcpy(entree, "DAY=FRI\nCLOCK=13:14:15\n");
    ei = 0;
    avancer(100);
    assert(rtc_memoire.year == 2026 && rtc_memoire.month == 10 && rtc_memoire.day == 9);
    assert(rtc_memoire.dayOfWeek == 6 && rtc_memoire.hours == 13 &&
           rtc_memoire.minutes == 14 && rtc_memoire.seconds == 15);
    rtc_lisible = true;
    strcpy(entree, "CLOCK=12:00:00\n");
    ei = 0;
    avancer(100);
    strcpy(entree, "LOG_INTERVAL=2\r\nTIMEOUT=2\r\n");
    ei = 0;
    avancer(100);
    assert(sauvegarde.log_interval == 2 && sauvegarde.timeout == 2);
    eeprom_ok = false;
    strcpy(entree, "LOG_INTERVAL=3\n");
    ei = 0;
    avancer(100);
    assert(sauvegarde.log_interval == 2);
    eeprom_ok = true;
    r = false;
    avancer(1800000);
    assert(lectures > avant);
    env_present = false;
    rtc_present = false;
    avancer(3000);
    assert(strstr(derniere_ligne, "2026-10-09T12:00:00"));
    /* Passer maintenance et verifier l'absence totale d'ecriture SD. */
    bouton(true);
    unsigned n = ecritures;
    avancer(6000);
    assert(ecritures == n && fermetures > 0);
    env_present = rtc_present = true;
    bouton(true);
    envoyer_gps();
    avancer(500);
    assert(ecritures > n);
    /* Economique : intervalle double, GPS une mesure sur deux. */
    bouton(false);
    envoyer_gps();
    avancer(500);
    n = ecritures;
    avancer(230000);
    assert(ecritures == n);
    avancer(11000);
    assert(ecritures == n + 1);
    assert(strstr(derniere_ligne, ";NA;NA\r\n"));
    puts("OK : vraie boucle, configuration sans acquisition, EEPROM refusee, inactivite, "
         "maintenance et economie.");
}
