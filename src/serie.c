/* Les interruptions conservent les octets pendant les lectures I2C/SPI.
 * La boucle principale reste seule responsable de leur interpretation.
 */
#include "serie.h"
#include "stm32l4xx_hal.h"
extern UART_HandleTypeDef huart2, GPS_UART_HANDLE;
typedef struct
{
    uint8_t donnees[1024], octet;
    volatile uint16_t tete, queue;
    volatile bool debordement;
} Reception;
static Reception console_rx, gps_rx;
void serie_init(void)
{
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_NVIC_SetPriority(UART4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
    HAL_UART_Receive_IT(&huart2, &console_rx.octet, 1);
    HAL_UART_Receive_IT(&GPS_UART_HANDLE, &gps_rx.octet, 1);
}
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
void UART4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&GPS_UART_HANDLE);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *u)
{
    Reception *r = u->Instance == UART4 ? &gps_rx : &console_rx;
    uint16_t prochain = (r->tete + 1) % sizeof r->donnees;
    if (prochain == r->queue)
        r->debordement = true;
    else
    {
        r->donnees[r->tete] = r->octet;
        r->tete = prochain;
    }
    HAL_UART_Receive_IT(u, &r->octet, 1);
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *u)
{
    Reception *r = u->Instance == UART4 ? &gps_rx : &console_rx;
    r->debordement = true;
    /* HAL a termine la reception en cas d'overrun, on la rearme. */
    HAL_UART_Receive_IT(u, &r->octet, 1);
}
bool serie_lire(bool gps, uint8_t *octet)
{
    Reception *r = gps ? &gps_rx : &console_rx;
    if (r->tete == r->queue)
        return false;
    *octet = r->donnees[r->queue];
    r->queue = (r->queue + 1) % sizeof r->donnees;
    return true;
}
bool serie_debordement(bool gps)
{
    Reception *r = gps ? &gps_rx : &console_rx;
    __disable_irq();
    bool perdu = r->debordement;
    r->debordement = false;
    if (perdu)
        r->queue = r->tete;
    __enable_irq();
    return perdu;
}
