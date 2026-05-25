/**
 * @file dac8830.c
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "dac8830.h"
#include "stm32l1xx_hal_def.h"
#include "stm32l1xx_hal_gpio.h"


bool
dac8830_Init(DAC8830_Handle_t * h, SPI_HandleTypeDef * hspi, GPIO_TypeDef * port, uint16_t pin)
{
    if ( !h || !hspi || !port )
    {
        return false;
    }

    h->hspi = hspi;
    h->cs_port = port;
    h->cs_pin = pin;
    h->init = true;

    if ( HAL_OK != dac8830_SetCode(h, 0U) )
    {
        h->hspi = NULL;
        h->cs_port = NULL;
        h->cs_pin = 0;
        h->init = false;
        return false;
    }

    return true;
}


HAL_StatusTypeDef
dac8830_SetCode(DAC8830_Handle_t * h, uint16_t code)
{
    if ( !h || !h->init || !h->hspi || !h->cs_port )
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef res;
    uint8_t payload[2] = {
        (uint8_t)(code >> 8),
        (uint8_t)(code & 0xFF)
    };

    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_RESET);
    res = HAL_SPI_Transmit(h->hspi, payload, sizeof(payload), DAC8830_SPI_TIMEOUT_MS);
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);
    
    return res;
}
