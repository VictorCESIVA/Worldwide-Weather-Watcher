#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include "stm32l4xx_hal.h"

/*
 * Grove Chainable RGB LED :
 * - Port Grove D6 du shield
 * - Signal1=D6=PB10 → CIN (horloge P9813)
 * - Signal2=D7=PA8  → DIN (données P9813)
 */
#define RGB_DATA_PORT GPIOA
#define RGB_DATA_PIN GPIO_PIN_8

#define RGB_CLK_PORT GPIOB
#define RGB_CLK_PIN GPIO_PIN_10

/*
 * Grove Light Sensor v1.3
 * Port Grove A2 du shield (PA4 = ADC1_IN9).
 */
#define LIGHT_ADC_CHANNEL ADC_CHANNEL_9
#define LIGHT_GPIO_PORT GPIOA
#define LIGHT_GPIO_PIN GPIO_PIN_4

/*
 * Grove Dual Button v1.0
 * À brancher sur un port digital Grove, par exemple D2.
 * À adapter selon le port choisi et le mapping réel.
 */
#define BUTTON1_PORT GPIOA
#define BUTTON1_PIN GPIO_PIN_10

#define BUTTON2_PORT GPIOB
#define BUTTON2_PIN GPIO_PIN_3

/*
 * Grove GPS Air530Z
 * Port Grove A0 du shield (PA0=UART4_TX, PA1=UART4_RX, AF8).
 * Seul UART libre sur un connecteur Grove unique (USART2=debug, USART1 split sur D2+D8).
 */

/*
 * Grove RTC v1.2 DS1307 + BME680
 * Tous deux sur I2C.
 */
#define GROVE_I2C_HANDLE hi2c1

/*
 * SD Card Shield Seeed v4.4 (famille v4)
 * SPI Arduino : D11/D12/D13 et CS sur D4.
 */
#ifdef SD_GROVE_BREAKOUT
/* Diagnostic avec deux adaptateurs Grove vers Dupont femelles :
 * D2 = CS (PA10), D3 = SCK (PB3), D4 = MOSI (PB5), D5 = MISO (PB4).
 * Tous les autres modules doivent etre debranches pendant cet essai.
 */
#define SD_CS_PORT GPIOA
#define SD_CS_PIN GPIO_PIN_10
#else
#define SD_CS_PORT GPIOB
#define SD_CS_PIN GPIO_PIN_5
#endif

#define SD_SPI_HANDLE hspi1

#endif
