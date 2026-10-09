/* EEPROM externe AT24C32, adresse 0x50, pages de 32 octets.
 * Deux copies : l'ancienne reste valide si l'alimentation coupe pendant l'ecriture.
 */
#include "parametres.h"
#include "board_pins.h"
#include <stddef.h>
#include <string.h>
extern I2C_HandleTypeDef GROVE_I2C_HANDLE;
#define EEPROM_ADDR (0x50 << 1)
#define SIGNATURE 0x33575731u
#define SLOT_SIZE 128u
typedef struct
{
    uint32_t signature, sequence;
    Parametres p;
    uint32_t crc;
} Bloc;
static uint32_t sequence;
static unsigned slot_actif = 1;
static uint32_t crc32(const void *p, size_t n)
{
    const uint8_t *b = p;
    uint32_t c = ~0u;
    while (n--)
    {
        c ^= *b++;
        for (unsigned k = 0; k < 8; ++k)
            c = (c >> 1) ^ ((c & 1) ? 0xEDB88320u : 0);
    }
    return ~c;
}
static bool lire(unsigned slot, Bloc *b)
{
    return HAL_I2C_Mem_Read(&GROVE_I2C_HANDLE, EEPROM_ADDR, slot * SLOT_SIZE, I2C_MEMADD_SIZE_16BIT,
                            (uint8_t *)b, sizeof *b, 50) == HAL_OK;
}
static bool valide(const Bloc *b)
{
    return b->signature == SIGNATURE && b->crc == crc32(&b->sequence, offsetof(Bloc, crc) - 4) &&
           parametres_valides(&b->p);
}
bool parametres_charger(Parametres *p)
{
    Bloc a, b;
    bool va = lire(0, &a) && valide(&a), vb = lire(1, &b) && valide(&b);
    if (!va && !vb)
    {
        parametres_defaut(p);
        return false;
    }
    slot_actif = vb && (!va || (int32_t)(b.sequence - a.sequence) > 0) ? 1 : 0;
    const Bloc *choisi = slot_actif ? &b : &a;
    sequence = choisi->sequence;
    *p = choisi->p;
    return true;
}
static bool ecrire(uint16_t adresse, const void *data, unsigned taille)
{
    const uint8_t *b = data;
    while (taille)
    {
        unsigned n = 32 - adresse % 32;
        if (n > taille)
            n = taille;
        if (HAL_I2C_Mem_Write(&GROVE_I2C_HANDLE, EEPROM_ADDR, adresse, I2C_MEMADD_SIZE_16BIT,
                              (uint8_t *)b, n, 50) != HAL_OK)
            return false;
        uint32_t debut = HAL_GetTick();
        while (HAL_I2C_IsDeviceReady(&GROVE_I2C_HANDLE, EEPROM_ADDR, 1, 1) != HAL_OK)
            if (HAL_GetTick() - debut > 20)
                return false;
        adresse += n;
        b += n;
        taille -= n;
    }
    return true;
}
bool parametres_sauver(const Parametres *p)
{
    if (!parametres_valides(p))
        return false;
    unsigned cible = 1 - slot_actif;
    uint16_t adresse = cible * SLOT_SIZE;
    Bloc nouveau = {SIGNATURE, sequence + 1, *p, 0}, controle;
    nouveau.crc = crc32(&nouveau.sequence, offsetof(Bloc, crc) - 4);
    uint32_t zero = 0;
    if (!ecrire(adresse, &zero, 4) ||
        !ecrire(adresse + 4, (uint8_t *)&nouveau + 4, sizeof nouveau - 4) ||
        !ecrire(adresse, &nouveau.signature, 4) || !lire(cible, &controle) || !valide(&controle) ||
        memcmp(&controle, &nouveau, sizeof nouveau))
        return false;
    sequence = nouveau.sequence;
    slot_actif = cible;
    return true;
}
