/**
 * @file dac8830.h
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef DAC8830_H
#define DAC8830_H

/*
What do i want from this driver?
I want it to: 
    +   initialize
    +   set output
    +   simple error handler (ret val)
*/


// --- INCLUDES ---

#include "stm32l152xe.h"
#include "stm32l1xx_hal.h"

#include <stdint.h>
#include <stdbool.h>


// --- DEFINES ---

#define DAC8830_SPI_TIMEOUT_MS 100U


// --- STRUCTS ---

typedef struct
{
    SPI_HandleTypeDef * hspi;
    GPIO_TypeDef *      cs_port;
    uint16_t            cs_pin;
    bool                init;
} DAC8830_Handle_t;


// --- FUNCTIONS ---

/**
 * @brief 
 * 
 * @param h 
 * @param hspi 
 * @param port 
 * @param pin 
 * @return true 
 * @return false 
 */
bool dac8830_Init(DAC8830_Handle_t * h, SPI_HandleTypeDef * hspi, GPIO_TypeDef * port, uint16_t pin);


/**
 * @brief 
 * 
 * @param h 
 * @param code 
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef dac8830_SetCode(DAC8830_Handle_t * h, uint16_t code);

#endif // DAC8830_H
