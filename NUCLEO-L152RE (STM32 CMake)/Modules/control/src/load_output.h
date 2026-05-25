#ifndef LOAD_OUTPUT_H
#define LOAD_OUTPUT_H

#include "stm32l1xx_hal.h"
#include <stdint.h>

HAL_StatusTypeDef LoadOutput_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
HAL_StatusTypeDef LoadOutput_SetDacCode(uint16_t code);
HAL_StatusTypeDef LoadOutput_SetZero(void);

#endif // LOAD_OUTPUT_H
