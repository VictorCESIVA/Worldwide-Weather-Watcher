#ifndef HAL_TEST_H
#define HAL_TEST_H
#include <stdint.h>
typedef struct
{
    int instance;
} UART_HandleTypeDef;
uint32_t HAL_GetTick(void);
typedef struct
{
    int instance;
} I2C_HandleTypeDef;
#define I2C_MEMADD_SIZE_16BIT 2
#define HAL_OK 0
#define HAL_ERROR 1
int HAL_I2C_Mem_Read(I2C_HandleTypeDef *, uint16_t, uint16_t, uint16_t, uint8_t *, uint16_t,
                     uint32_t);
int HAL_I2C_Mem_Write(I2C_HandleTypeDef *, uint16_t, uint16_t, uint16_t, uint8_t *, uint16_t,
                      uint32_t);
int HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *, uint16_t, uint32_t, uint32_t);
#endif
