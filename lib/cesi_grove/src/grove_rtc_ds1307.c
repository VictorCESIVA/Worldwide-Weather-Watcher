/* API de la base CESI conservee. DS1307 (0x68) ou Grove v2 PCF85063TP (0x51).
 * PCF85063 : registres verifies dans la documentation du fabricant NXP.
 */
#include "grove_rtc_ds1307.h"
#include "board_pins.h"
#include "parametres.h"
extern I2C_HandleTypeDef GROVE_I2C_HANDLE;
static uint16_t adresse;
static bool pcf;
static uint8_t bcd(uint8_t v)
{
    return (v >> 4) * 10 + (v & 15);
}
static uint8_t dec(uint8_t v)
{
    return ((v / 10) << 4) | (v % 10);
}
static bool lire(uint8_t reg, uint8_t *data, uint16_t n)
{
    return HAL_I2C_Mem_Read(&GROVE_I2C_HANDLE, adresse, reg, I2C_MEMADD_SIZE_8BIT, data, n, 20) ==
           HAL_OK;
}
static bool ecrire(uint8_t reg, uint8_t *data, uint16_t n)
{
    return HAL_I2C_Mem_Write(&GROVE_I2C_HANDLE, adresse, reg, I2C_MEMADD_SIZE_8BIT, data, n, 20) ==
           HAL_OK;
}
bool GroveRTC_Init(void)
{
    adresse = 0x68 << 1;
    pcf = false;
    if (HAL_I2C_IsDeviceReady(&GROVE_I2C_HANDLE, adresse, 1, 20) == HAL_OK)
        return true;
    adresse = 0x51 << 1;
    pcf = true;
    return HAL_I2C_IsDeviceReady(&GROVE_I2C_HANDLE, adresse, 1, 20) == HAL_OK;
}
bool GroveRTC_GetDateTime(RTC_DateTime *d)
{
    if (!d || (!adresse && !GroveRTC_Init()))
        return false;
    uint8_t v[7], controle = 0;
    if (!lire(pcf ? 4 : 0, v, 7))
        return false;
    if (v[0] & 0x80)
        return false; /* CH DS1307 ou OS PCF : heure non fiable. */
    if (pcf && (!lire(0, &controle, 1) || (controle & 0x20)))
        return false;
    v[0] &= 0x7F;
    v[1] &= 0x7F;
    d->seconds = bcd(v[0]);
    d->minutes = bcd(v[1]);
    if ((pcf && (controle & 2)) || (!pcf && (v[2] & 0x40)))
    {
        uint8_t h = bcd(v[2] & 0x1F);
        if (h < 1 || h > 12)
            return false;
        d->hours = h % 12 + ((v[2] & 0x20) ? 12 : 0);
    }
    else
        d->hours = bcd(v[2] & 0x3F);
    if (pcf)
    {
        d->day = bcd(v[3] & 0x3F);
        d->dayOfWeek = (v[4] & 7) + 1;
        d->month = bcd(v[5] & 0x1F);
    }
    else
    {
        d->dayOfWeek = v[3] & 7;
        d->day = bcd(v[4] & 0x3F);
        d->month = bcd(v[5] & 0x1F);
    }
    d->year = 2000 + bcd(v[6]);
    /* Refuser toute donnee BCD mal formee (hors champ weekday binaire). */
    for (unsigned i = 0; i < 7; ++i)
    {
        if (i == (pcf ? 4u : 3u))
            continue;
        uint8_t x = v[i];
        if (i == 2)
            x &= 0x1F;
        else if (i == 5)
            x &= 0x1F;
        if ((x & 15) > 9 || (x >> 4) > 9)
            return false;
    }
    return date_valide(d);
}
bool GroveRTC_SetDateTime(const RTC_DateTime *d)
{
    if (!d || !date_valide(d) || (!adresse && !GroveRTC_Init()))
        return false;
    uint8_t v[7] = {dec(d->seconds), dec(d->minutes),    dec(d->hours), 0, 0,
                    dec(d->month),   dec(d->year - 2000)};
    if (pcf)
    {
        uint8_t controle;
        if (!lire(0, &controle, 1))
            return false;
        uint8_t stop = controle | 0x20;
        if (!ecrire(0, &stop, 1))
            return false;
        v[3] = dec(d->day);
        v[4] = d->dayOfWeek - 1;
        bool ok = ecrire(4, v, 7);
        controle = (controle & ~0x22) | 1; /* 24h, horloge active, quartz Grove 12.5 pF. */
        return ecrire(0, &controle, 1) && ok;
    }
    v[3] = d->dayOfWeek;
    v[4] = dec(d->day);
    return ecrire(0, v, 7);
}
