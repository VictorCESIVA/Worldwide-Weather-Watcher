#include "setup.h"
#include "board_pins.h"
#include "grove_button.h"
#include "grove_light.h"
#include "grove_rgb.h"
#include "grove_rtc_ds1307.h"
#include "grove_bme680.h"
#include "grove_gps_air530z.h"
#include "sd_logger.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef GPS_UART_HANDLE;
/* Resultats aussi lisibles au debogueur si la liaison serie est indisponible. */
volatile bool diag_rtc, diag_meteo, diag_sd, diag_rtc_valide, diag_meteo_valide;
volatile RTC_DateTime diag_date;
volatile Env_Data diag_environnement;
volatile uint32_t diag_cycles;
volatile uint8_t diag_rtc_registres[11];
int main(void)
{
    Global_Init();
    GroveButton_Init();
    GroveRGB_Init();
    GroveLight_Init();
    GroveGPS_Init();
    printf("\r\n3WW diagnostic : NUCLEO-L476RG\r\nI2C :");
    for (unsigned int a = 8; a < 120; ++a)
        if (HAL_I2C_IsDeviceReady(&hi2c1, a << 1, 1, 10) == HAL_OK)
            printf(" 0x%02X", a);
    uint8_t id = 0;
    HAL_I2C_Mem_Read(&hi2c1, 0x76 << 1, 0xD0, I2C_MEMADD_SIZE_8BIT, &id, 1, 100);
    printf("\r\nIdentifiant meteo 0x76/D0=0x%02X\r\n", id);
    diag_rtc = GroveRTC_Init();
    diag_meteo = GroveBME680_Init();
    diag_sd = SDLogger_Init();
    printf("\r\nRTC=%d METEO=%d SD=%d\r\n", diag_rtc, diag_meteo, diag_sd);
    while (1)
    {
        RTC_DateTime t;
        Env_Data e;
        printf("bouton rouge=%d vert=%d lumi=%u", GroveButton1_IsPressed(),
               GroveButton2_IsPressed(), GroveLight_ReadRaw());
        diag_rtc_valide = GroveRTC_GetDateTime(&t);
        if (diag_rtc_valide)
            diag_date = t;
        uint8_t registre[11];
        if (HAL_I2C_Mem_Read(&hi2c1, 0x51 << 1, 0, I2C_MEMADD_SIZE_8BIT, registre, 11, 20) ==
            HAL_OK)
            for (unsigned k = 0; k < 11; ++k)
                diag_rtc_registres[k] = registre[k];
        if (diag_rtc_valide)
            printf(" RTC=%04u-%02u-%02u %02u:%02u:%02u", t.year, t.month, t.day, t.hours, t.minutes,
                   t.seconds);
        diag_meteo_valide = GroveBME680_ReadEnvironment(&e);
        if (diag_meteo_valide)
            diag_environnement = e;
        ++diag_cycles;
        if (diag_meteo_valide)
            printf(" T=%.1f H=%.1f P=%.1f", (double)e.temperature_c, (double)e.humidity_percent,
                   (double)e.pressure_hpa);
        printf("\r\nGPS brut (1 s):\r\n");
        uint32_t start = HAL_GetTick();
        uint8_t c;
        while (HAL_GetTick() - start < 1000)
            if (HAL_UART_Receive(&GPS_UART_HANDLE, &c, 1, 2) == HAL_OK)
                putchar(c);
        GroveRGB_SetColor(0, 20, 0);
    }
}
