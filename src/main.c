#include "setup.h"
#include "station.h"
#include "stm32l4xx_hal.h"
int main(void)
{
    Global_Init();
    station_init();
    while (1)
    {
        station_boucle();
        HAL_Delay(1);
    }
}
