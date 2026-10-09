#include "parametres.h"
#include "stm32l4xx_hal.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
I2C_HandleTypeDef hi2c1;
static uint8_t memoire[4096];
static int budget = -1;
static bool presente = true;
static unsigned tick;
uint32_t HAL_GetTick(void)
{
    return ++tick;
}
int HAL_I2C_Mem_Read(I2C_HandleTypeDef *h, uint16_t a, uint16_t adr, uint16_t mode, uint8_t *b,
                     uint16_t n, uint32_t t)
{
    (void)h;
    (void)t;
    assert(a == (0x50 << 1));
    assert(mode == 2);
    assert(adr + n <= sizeof memoire);
    if (!presente)
        return HAL_ERROR;
    memcpy(b, memoire + adr, n);
    return HAL_OK;
}
int HAL_I2C_Mem_Write(I2C_HandleTypeDef *h, uint16_t a, uint16_t adr, uint16_t mode, uint8_t *b,
                      uint16_t n, uint32_t t)
{
    (void)h;
    (void)t;
    assert(a == (0x50 << 1));
    assert(mode == 2);
    assert(adr % 32 + n <= 32);
    if (!presente)
        return HAL_ERROR;
    for (unsigned i = 0; i < n; ++i)
    {
        if (budget == 0)
            return HAL_ERROR;
        memoire[adr + i] = b[i];
        if (budget > 0)
            --budget;
    }
    return HAL_OK;
}
int HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *h, uint16_t a, uint32_t n, uint32_t t)
{
    (void)h;
    (void)a;
    (void)n;
    (void)t;
    return presente ? HAL_OK : HAL_ERROR;
}
int main(void)
{
    memset(memoire, 255, sizeof memoire);
    Parametres ancien, nouveau, lu;
    assert(!parametres_charger(&ancien));
    assert(ancien.log_interval == 10);
    assert(parametres_sauver(&ancien));
    nouveau = ancien;
    nouveau.log_interval = 3;
    uint8_t image[4096];
    memcpy(image, memoire, sizeof image);
    for (int coupe = 0; coupe < 90; ++coupe)
    {
        memcpy(memoire, image, sizeof image);
        budget = -1;
        assert(parametres_charger(&lu));
        budget = coupe;
        bool succes = parametres_sauver(&nouveau);
        budget = -1;
        assert(parametres_charger(&lu));
        assert(!memcmp(&lu, succes ? &nouveau : &ancien, sizeof lu));
    }
    memcpy(memoire, image, sizeof image);
    budget = -1;
    assert(parametres_charger(&lu));
    assert(parametres_sauver(&nouveau));
    assert(parametres_charger(&lu));
    assert(lu.log_interval == 3);
    memoire[128 + 12] ^= 1; /* Corrompre la copie la plus recente. */
    assert(parametres_charger(&lu));
    assert(lu.log_interval == 10);
    presente = false;
    assert(!parametres_charger(&lu));
    assert(!parametres_sauver(&nouveau));
    puts("OK : EEPROM, pages, CRC, absence et coupures a chaque octet de la sauvegarde.");
}
