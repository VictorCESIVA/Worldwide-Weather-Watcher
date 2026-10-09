#include "grove_gps_air530z.h"
#include "board_pins.h"
#include <string.h>
#include <stdlib.h>

UART_HandleTypeDef GPS_UART_HANDLE;

void GroveGPS_Init(void)
{
    /*
     * Port Grove A0 : Signal1=PA0=GPS_TX, Signal2=PA1=GPS_RX.
     * Le module Air530Z émet sur Signal1 (PA0).
     * Pin swap UART4 activé : PA0=UART4_RX, PA1=UART4_TX.
     * L'init GPIO est centralisée dans HAL_UART_MspInit (setup.c).
     */
    GPS_UART_HANDLE.Instance = UART4;
    GPS_UART_HANDLE.Init.BaudRate = 9600;
    GPS_UART_HANDLE.Init.WordLength = UART_WORDLENGTH_8B;
    GPS_UART_HANDLE.Init.StopBits = UART_STOPBITS_1;
    GPS_UART_HANDLE.Init.Parity = UART_PARITY_NONE;
    GPS_UART_HANDLE.Init.Mode = UART_MODE_TX_RX;
    GPS_UART_HANDLE.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    GPS_UART_HANDLE.Init.OverSampling = UART_OVERSAMPLING_16;
    GPS_UART_HANDLE.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    GPS_UART_HANDLE.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_SWAP_INIT;
    GPS_UART_HANDLE.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;

    HAL_UART_Init(&GPS_UART_HANDLE);
}

bool GroveGPS_ReadLine(char *buffer, unsigned int max_len, unsigned int timeout_ms)
{
    if (buffer == 0 || max_len == 0)
        return false;

    unsigned int count = 0;
    uint8_t c = 0;
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms && count < (max_len - 1))
    {
        if (HAL_UART_Receive(&GPS_UART_HANDLE, &c, 1, 10) == HAL_OK)
        {
            if (c == '\n')
            {
                buffer[count] = '\0';
                return true;
            }

            if (c != '\r')
            {
                buffer[count++] = (char)c;
            }
        }
    }

    buffer[count] = '\0';
    return count > 0;
}

/* Parseur partage avec l'application, champs vides et checksum verifies. */
#include "nmea.h"
bool GroveGPS_ReadGGA(GPS_Data *gps, unsigned int timeout_ms)
{
    if (!gps)
        return false;
    gps->valid = false;
    uint32_t debut = HAL_GetTick();
    char ligne[96];
    while (HAL_GetTick() - debut < timeout_ms)
    {
        unsigned reste = timeout_ms - (HAL_GetTick() - debut);
        if (GroveGPS_ReadLine(ligne, sizeof ligne, reste) && nmea_gga(ligne, gps))
            return true;
    }
    return false;
}
