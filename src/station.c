/* Boucle simple : boutons, console, acquisition, LED. Pas de tas ni de RTOS. */
#include "station.h"
#include "grove_button.h"
#include "grove_rgb.h"
#include "grove_light.h"
#include "grove_bme680.h"
#include "grove_rtc_ds1307.h"
#include "grove_gps_air530z.h"
#include "sd_logger.h"
#include "nmea.h"
#include "serie.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
extern UART_HandleTypeDef huart2, GPS_UART_HANDLE;
#ifndef BUILD_NUMBER
#define BUILD_NUMBER 0
#endif
static Parametres config;
static Mode mode, precedent = STANDARD;
static Capteur lumi, temp, humidite, pression;
static RTC_DateTime date;
/* Dernier reglage accepte, utilise seulement pendant la configuration.
 * Les mesures horodatees restent issues d'une vraie lecture de la RTC.
 */
static RTC_DateTime reglage_rtc;
static bool reglage_rtc_valide;
static GPS_Data gps;
static bool rtc_ok, gps_ok, env_fini, lumi_fini, rtc_fini, gps_fini, cycle, sd_montee;
static uint8_t rtc_timeouts, gps_timeouts;
static unsigned cycle_eco;
static uint32_t dernier_cycle, debut_cycle, dernier_essai, activite;
static Erreur erreur_sd;
static char commande[96], gps_ligne[96];
static unsigned commande_len, gps_len;
static bool commande_trop_longue;
static const char *noms[] = {"standard", "configuration", "maintenance", "economique"};

typedef struct
{
    bool brut, stable, declenche;
    uint32_t changement, debut;
} Bouton;
static Bouton rouge, vert;
static bool appui_long(Bouton *b, bool presse, uint32_t maintenant)
{
    if (presse != b->brut)
    {
        b->brut = presse;
        b->changement = maintenant;
    }
    if (b->stable != b->brut && maintenant - b->changement >= 30)
    {
        b->stable = b->brut;
        b->debut = maintenant;
        b->declenche = false;
    }
    if (b->stable && !b->declenche && maintenant - b->debut >= 5000)
    {
        b->declenche = true;
        return true;
    }
    return false;
}
static void changer_mode(Mode nouveau)
{
    if (nouveau == mode)
        return;
    if (nouveau == MAINTENANCE)
        precedent = mode;
    if (sd_montee)
    {
        SDLogger_Unmount();
        sd_montee = false;
    }
    mode = nouveau;
    cycle = false;
    cycle_eco = 0;
    dernier_cycle = HAL_GetTick();
    /* Prochaine acquisition immediate, sans toucher au parametre EEPROM. */
    dernier_cycle -= mode == MAINTENANCE
                         ? 1000u
                         : (uint32_t)config.log_interval * 60000u * (mode == ECONOMIQUE ? 2u : 1u);
    erreur_sd = ERREUR_AUCUNE;
    printf("MODE=%s\r\n", noms[mode]);
    if (mode == MAINTENANCE)
        printf("SD fermee : retrait autorise.\r\n");
}
static void executer_commande(void)
{
    if (!strcmp(commande, "VERSION"))
    {
        printf("3WW 1.0.0 LOT=CESI-2026 BUILD=%u\r\n", (unsigned)BUILD_NUMBER);
        return;
    }
    if (mode != CONFIGURATION)
    {
        printf("ERREUR : configuration requise.\r\n");
        return;
    }
    Parametres nouveau = config;
    if (!strcmp(commande, "RESET"))
        parametres_defaut(&nouveau);
    else if (!strncmp(commande, "CLOCK=", 6) || !strncmp(commande, "DATE=", 5) ||
             !strncmp(commande, "DAY=", 4))
    {
        RTC_DateTime d;
        /* Ne pas perdre DATE si la relecture entre deux commandes echoue. */
        if (!GroveRTC_GetDateTime(&d))
            d = reglage_rtc_valide ? reglage_rtc : (RTC_DateTime){0, 0, 0, 1, 1, 1, 2000};
        if (!regler_rtc(&d, commande) || !GroveRTC_SetDateTime(&d))
            printf("ERREUR RTC ou format.\r\n");
        else
        {
            reglage_rtc = d;
            reglage_rtc_valide = true;
            rtc_ok = true;
            rtc_timeouts = 0;
            printf("OK RTC\r\n");
        }
        return;
    }
    else if (!parametres_modifier(&nouveau, commande))
    {
        printf("ERREUR : commande, domaine ou seuils.\r\n");
        return;
    }
    if (!parametres_sauver(&nouveau))
    {
        printf("ERREUR EEPROM : parametres non modifies.\r\n");
        return;
    }
    config = nouveau;
    printf("OK EEPROM\r\n");
}
static void console(uint32_t maintenant)
{
    uint8_t c;
    if (serie_debordement(false))
    {
        commande_len = 0;
        commande_trop_longue = true;
    }
    while (serie_lire(false, &c))
    {
        activite = maintenant;
        if (c == '\r' || c == '\n')
        {
            if (commande_trop_longue)
                printf("ERREUR : ligne trop longue.\r\n");
            else if (commande_len)
            {
                commande[commande_len] = 0;
                executer_commande();
            }
            commande_len = 0;
            commande_trop_longue = false;
        }
        else if (c == 8 || c == 127)
        {
            if (commande_len)
                --commande_len;
        }
        else if (c >= 32 && c <= 126)
        {
            if (commande_len < sizeof commande - 1)
                commande[commande_len++] = c;
            else
                commande_trop_longue = true;
        }
    }
}
static void recevoir_gps(void)
{
    uint8_t c;
    if (serie_debordement(true))
        gps_len = 0;
    while (serie_lire(true, &c))
    {
        if (c == '$')
        {
            gps_len = 0;
            gps_ligne[gps_len++] = c;
        }
        else if (c == '\n')
        {
            gps_ligne[gps_len] = 0;
            if (cycle && !gps_fini && nmea_gga(gps_ligne, &gps))
            {
                gps_ok = true;
                gps_fini = true;
                gps_timeouts = 0;
            }
            gps_len = 0;
        }
        else if (c != '\r' && gps_len)
        {
            if (gps_len < sizeof gps_ligne - 1)
                gps_ligne[gps_len++] = c;
            else
                gps_len = 0;
        }
    }
}
static bool convertir(float v, int16_t *sortie)
{
    if (!isfinite(v) || v < -3276.8f || v > 3276.7f)
        return false;
    float x = v * 10.0f;
    *sortie = (int16_t)(x + (x >= 0 ? 0.5f : -0.5f));
    return true;
}
static void traiter_env(const Env_Data *e)
{
    int16_t t = 0, h = 0, p = 0;
    bool t_phys =
        convertir(e->temperature_c, &t) && e->temperature_c >= -40 && e->temperature_c <= 85;
    if (config.temp_air)
    {
        if (t_phys && e->temperature_c >= config.min_temp_air &&
            e->temperature_c <= config.max_temp_air)
            mesure_ajouter(&temp, t);
        else
            mesure_echouer(&temp, true);
    }
    if (config.hygr)
    {
        if (!t_phys || !convertir(e->humidity_percent, &h) || e->humidity_percent < 0 ||
            e->humidity_percent > 100)
            mesure_echouer(&humidite, true);
        else if (e->temperature_c < config.hygr_mint || e->temperature_c > config.hygr_maxt)
        {
            humidite.valide = false;
            humidite.incoherente = false;
            humidite.timeouts = 0;
        }
        else
            mesure_ajouter(&humidite, h);
    }
    if (config.pressure)
    {
        if (convertir(e->pressure_hpa, &p) && e->pressure_hpa >= config.pressure_min &&
            e->pressure_hpa <= config.pressure_max)
            mesure_ajouter(&pression, p);
        else
            mesure_echouer(&pression, true);
    }
}
static void demarrer_cycle(uint32_t maintenant)
{
    cycle = true;
    debut_cycle = maintenant;
    dernier_cycle = maintenant;
    dernier_essai = maintenant - 1000;
    rtc_fini = false;
    rtc_ok = false;
    gps_ok = false;
    gps_len = 0;
    gps_fini = mode == ECONOMIQUE && cycle_eco % 2 == 1;
    if (mode == ECONOMIQUE)
        ++cycle_eco;
    lumi_fini = !config.lumin;
    env_fini = !config.temp_air && !config.hygr && !config.pressure;
    lumi.valide = temp.valide = humidite.valide = pression.valide = false;
}
static void texte_mesure(const Capteur *c, bool actif, bool dixiemes, char *s, size_t n)
{
    if (!actif || !c->valide)
    {
        snprintf(s, n, "NA");
        return;
    }
    int v = mesure_moyenne(c);
    if (dixiemes)
        snprintf(s, n, "%s%d.%d", v < 0 ? "-" : "", v < 0 ? -v / 10 : v / 10,
                 v < 0 ? -v % 10 : v % 10);
    else
        snprintf(s, n, "%d", v);
}
static void terminer_cycle(void)
{
    char lt[16], tt[16], ht[16], pt[16], dt[32], position[48], ligne[256], nom[32];
    texte_mesure(&lumi, config.lumin, false, lt, sizeof lt);
    texte_mesure(&temp, config.temp_air, true, tt, sizeof tt);
    texte_mesure(&humidite, config.hygr, true, ht, sizeof ht);
    texte_mesure(&pression, config.pressure, true, pt, sizeof pt);
    if (rtc_ok)
        snprintf(dt, sizeof dt, "%04u-%02u-%02uT%02u:%02u:%02u", date.year, date.month, date.day,
                 date.hours, date.minutes, date.seconds);
    else
        strcpy(dt, "NA");
    if (gps_ok)
    {
        int32_t lat = (int32_t)(gps.latitude * 1000000.0 + (gps.latitude >= 0 ? 0.5 : -0.5));
        int32_t lon = (int32_t)(gps.longitude * 1000000.0 + (gps.longitude >= 0 ? 0.5 : -0.5));
        uint32_t a = lat < 0 ? (uint32_t)-lat : (uint32_t)lat;
        uint32_t b = lon < 0 ? (uint32_t)-lon : (uint32_t)lon;
        snprintf(position, sizeof position, "%s%lu.%06lu;%s%lu.%06lu", lat < 0 ? "-" : "",
                 (unsigned long)(a / 1000000), (unsigned long)(a % 1000000), lon < 0 ? "-" : "",
                 (unsigned long)(b / 1000000), (unsigned long)(b % 1000000));
    }
    else
        strcpy(position, "NA;NA");
    int moyenne_lumi = mesure_moyenne(&lumi);
    const char *niveau = !lumi.valide
                             ? "NA"
                             : (moyenne_lumi < config.lumin_low
                                    ? "faible"
                                    : (moyenne_lumi > config.lumin_high ? "forte" : "moyenne"));
    int len = snprintf(ligne, sizeof ligne, "%s;%s;%s;%s;%s;%s;%s\r\n", dt, tt, ht, pt, lt, niveau,
                       position);
    printf("%s", ligne);
    if (mode != MAINTENANCE && rtc_ok && len > 0 && len < (int)sizeof ligne)
    {
        snprintf(nom, sizeof nom, "%02u%02u%02u_0.LOG", date.year % 100, date.month, date.day);
        if (!sd_montee)
            sd_montee = SDLogger_Mount();
        if (!sd_montee)
            erreur_sd = ERREUR_SD;
        else if (!SDLogger_Append(nom, ligne, config.file_max_size))
        {
            erreur_sd = SDLogger_GetResult() == SDLOGGER_FULL ? ERREUR_SD_PLEINE : ERREUR_SD;
            SDLogger_Unmount();
            sd_montee = false;
        }
        else
            erreur_sd = ERREUR_AUCUNE;
    }
    cycle = false;
}
static void acquisition(uint32_t maintenant)
{
    if (mode == CONFIGURATION)
        return;
    uint32_t intervalle = mode == MAINTENANCE ? 1000u
                                              : (uint32_t)config.log_interval * 60000u *
                                                    (mode == ECONOMIQUE ? 2u : 1u);
    if (!cycle && maintenant - dernier_cycle >= intervalle)
        demarrer_cycle(maintenant);
    if (!cycle)
        return;
    if (maintenant - dernier_essai >= 250)
    {
        dernier_essai = maintenant;
        if (!rtc_fini && GroveRTC_GetDateTime(&date) && date_valide(&date))
        {
            rtc_fini = true;
            rtc_ok = true;
            rtc_timeouts = 0;
        }
        if (!lumi_fini)
        {
            uint16_t valeur = GroveLight_ReadRaw();
            if (valeur != 0xFFFF)
            {
                mesure_ajouter(&lumi, (int16_t)(valeur * 1023u / 4095u));
                lumi_fini = true;
            }
        }
        if (!env_fini)
        {
            Env_Data e;
            if (GroveBME680_ReadEnvironment(&e))
            {
                traiter_env(&e);
                env_fini = true;
            }
        }
    }
    if (HAL_GetTick() - debut_cycle >= (uint32_t)config.timeout * 1000u)
    {
        if (!rtc_fini)
        {
            rtc_fini = true;
            if (rtc_timeouts < 2)
                ++rtc_timeouts;
        }
        if (!gps_fini)
        {
            gps_fini = true;
            if (gps_timeouts < 2)
                ++gps_timeouts;
        }
        if (!lumi_fini)
        {
            lumi_fini = true;
            mesure_echouer(&lumi, false);
        }
        if (!env_fini)
        {
            env_fini = true;
            if (config.temp_air)
                mesure_echouer(&temp, false);
            if (config.hygr)
                mesure_echouer(&humidite, false);
            if (config.pressure)
                mesure_echouer(&pression, false);
        }
    }
    if (rtc_fini && gps_fini && lumi_fini && env_fini)
        terminer_cycle();
}
static Erreur erreur_courante(void)
{
    if (mode == CONFIGURATION)
        return ERREUR_AUCUNE;
    if (rtc_timeouts >= 2)
        return ERREUR_RTC;
    if (gps_timeouts >= 2)
        return ERREUR_GPS;
    if ((config.lumin && lumi.timeouts >= 2) || (config.temp_air && temp.timeouts >= 2) ||
        (config.hygr && humidite.timeouts >= 2) || (config.pressure && pression.timeouts >= 2))
        return ERREUR_CAPTEUR;
    if ((config.temp_air && temp.incoherente) || (config.hygr && humidite.incoherente) ||
        (config.pressure && pression.incoherente))
        return ERREUR_INCOHERENTE;
    return erreur_sd;
}
static void led(uint32_t maintenant)
{
    static int ancien = -1;
    Erreur e = erreur_courante();
    int couleur;
    if (e == ERREUR_AUCUNE)
        couleur = mode == STANDARD ? 1 : mode == CONFIGURATION ? 2 : mode == MAINTENANCE ? 3 : 4;
    else
    {
        uint32_t rouge_ms = (e == ERREUR_INCOHERENTE || e == ERREUR_SD) ? 333 : 500;
        couleur = maintenant % 1000 < rouge_ms
                      ? 5
                      : (e == ERREUR_RTC                                    ? 4
                         : e == ERREUR_GPS                                  ? 2
                         : (e == ERREUR_CAPTEUR || e == ERREUR_INCOHERENTE) ? 1
                                                                            : 6);
    }
    if (couleur == ancien)
        return;
    ancien = couleur;
    static const uint8_t rgb[][3] = {{0, 0, 0},   {0, 255, 0}, {255, 255, 0},  {255, 128, 0},
                                     {0, 0, 255}, {255, 0, 0}, {255, 255, 255}};
    GroveRGB_SetColor(rgb[couleur][0], rgb[couleur][1], rgb[couleur][2]);
}
void station_init(void)
{
    reglage_rtc_valide = false;
    GroveButton_Init();
    GroveRGB_Init();
    GroveLight_Init();
    GroveGPS_Init();
    serie_init();
    bool sauve = parametres_charger(&config);
    mode = GroveButton1_IsPressed() ? CONFIGURATION : STANDARD;
    rouge.brut = rouge.stable = GroveButton1_IsPressed();
    rouge.declenche = true;
    activite = HAL_GetTick();
    dernier_cycle = activite - (uint32_t)config.log_interval * 60000u;
    printf("\r\n3WW 1.0.0 LOT=CESI-2026 BUILD=%u MODE=%s\r\n", (unsigned)BUILD_NUMBER, noms[mode]);
    printf("PARAMETRES=%s LOG_INTERVAL=%ld FILE_MAX_SIZE=%ld TIMEOUT=%ld\r\n",
           sauve ? "EEPROM" : "defauts (EEPROM absente ou vierge)", (long)config.log_interval,
           (long)config.file_max_size, (long)config.timeout);
    printf(
        "date;temperature_C;humidite_pct;pression_hPa;lumi_0_1023;niveau;latitude;longitude\r\n");
}
void station_boucle(void)
{
    uint32_t maintenant = HAL_GetTick();
    bool r = appui_long(&rouge, GroveButton1_IsPressed(), maintenant);
    bool v = appui_long(&vert, GroveButton2_IsPressed(), maintenant);
    if (r)
        changer_mode(mode_apres_appui(mode, precedent, true));
    else if (v)
        changer_mode(mode_apres_appui(mode, precedent, false));
    console(maintenant);
    if (mode == CONFIGURATION && maintenant - activite >= 1800000u)
        changer_mode(STANDARD);
    recevoir_gps();
    acquisition(maintenant);
    led(HAL_GetTick());
}
