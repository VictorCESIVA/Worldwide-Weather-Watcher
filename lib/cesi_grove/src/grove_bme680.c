/* Adaptation du pilote Bosch officiel : la calibration reste dans la bibliotheque. */
#include "grove_bme680.h"
#include "board_pins.h"
#include "bme68x.h"
#include "bme280.h"
extern I2C_HandleTypeDef GROVE_I2C_HANDLE;
static struct bme68x_dev bme;
static struct bme68x_conf conf;
static struct bme280_dev bme280;
static struct bme280_settings reglages280;
static bool modele280;
static uint8_t adresse;
static bool initialise;

static BME68X_INTF_RET_TYPE lire(uint8_t reg, uint8_t *data, uint32_t len, void *ctx)
{
    uint8_t a = *(uint8_t *)ctx;
    return HAL_I2C_Mem_Read(&GROVE_I2C_HANDLE, a << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 20) ==
                   HAL_OK
               ? 0
               : -1;
}
static BME68X_INTF_RET_TYPE ecrire(uint8_t reg, const uint8_t *data, uint32_t len, void *ctx)
{
    uint8_t a = *(uint8_t *)ctx;
    return HAL_I2C_Mem_Write(&GROVE_I2C_HANDLE, a << 1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data,
                             len, 20) == HAL_OK
               ? 0
               : -1;
}
static void attendre(uint32_t us, void *ctx)
{
    (void)ctx;
    HAL_Delay((us + 999) / 1000);
}
bool GroveBME680_ReadChipId(uint8_t *id)
{
    return id && lire(0xD0, id, 1, &adresse) == 0;
}
bool GroveBME680_Init(void)
{
    initialise = false;
    bme.intf = BME68X_I2C_INTF;
    bme.read = lire;
    bme.write = ecrire;
    bme.delay_us = attendre;
    bme.intf_ptr = &adresse;
    bme.amb_temp = 25;
    for (adresse = 0x76; adresse <= 0x77; ++adresse)
    {
        uint8_t id = 0;
        if (!GroveBME680_ReadChipId(&id))
            continue;
        modele280 = id == 0x60;
        if (modele280)
        {
            bme280.intf = BME280_I2C_INTF;
            bme280.read = lire;
            bme280.write = ecrire;
            bme280.delay_us = attendre;
            bme280.intf_ptr = &adresse;
            if (bme280_init(&bme280) != BME280_OK)
                continue;
            reglages280.osr_t = BME280_OVERSAMPLING_2X;
            reglages280.osr_p = BME280_OVERSAMPLING_2X;
            reglages280.osr_h = BME280_OVERSAMPLING_2X;
            reglages280.filter = BME280_FILTER_COEFF_OFF;
            if (bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &reglages280, &bme280) !=
                BME280_OK)
                return false;
            initialise = true;
            return true;
        }
        if (bme68x_init(&bme) != BME68X_OK)
            continue;
        conf.filter = BME68X_FILTER_OFF;
        conf.odr = BME68X_ODR_NONE;
        conf.os_hum = BME68X_OS_2X;
        conf.os_temp = BME68X_OS_2X;
        conf.os_pres = BME68X_OS_2X;
        struct bme68x_heatr_conf h = {0};
        h.enable = BME68X_DISABLE;
        if (bme68x_set_conf(&conf, &bme) != BME68X_OK ||
            bme68x_set_heatr_conf(BME68X_FORCED_MODE, &h, &bme) != BME68X_OK)
            return false;
        initialise = true;
        return true;
    }
    return false;
}
bool GroveBME680_ReadEnvironment(Env_Data *d)
{
    struct bme68x_data raw;
    uint8_t count = 0;
    if (!d || (!initialise && !GroveBME680_Init()))
        return false;
    if (modele280)
    {
        struct bme280_data r;
        uint32_t delai;
        if (bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &bme280) != BME280_OK)
        {
            initialise = false;
            return false;
        }
        if (bme280_cal_meas_delay(&delai, &reglages280) != BME280_OK)
            return false;
        attendre(delai, NULL);
        if (bme280_get_sensor_data(BME280_ALL, &r, &bme280) != BME280_OK)
        {
            initialise = false;
            return false;
        }
        d->temperature_c = (float)r.temperature;
        d->humidity_percent = (float)r.humidity;
        d->pressure_hpa = (float)(r.pressure / 100.0);
        d->gas_resistance_ohm = 0;
        return true;
    }
    if (bme68x_set_op_mode(BME68X_FORCED_MODE, &bme) != BME68X_OK)
    {
        initialise = false;
        return false;
    }
    attendre(bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme), NULL);
    if (bme68x_get_data(BME68X_FORCED_MODE, &raw, &count, &bme) != BME68X_OK || !count)
        return false;
#ifdef BME68X_USE_FPU
    d->temperature_c = raw.temperature;
    d->pressure_hpa = raw.pressure / 100.0f;
    d->humidity_percent = raw.humidity;
#else
    d->temperature_c = raw.temperature / 100.0f;
    d->pressure_hpa = raw.pressure / 100.0f;
    d->humidity_percent = raw.humidity / 1000.0f;
#endif
    d->gas_resistance_ohm = 0; /* Le chauffage gaz est desactive : hors perimetre. */
    return true;
}
