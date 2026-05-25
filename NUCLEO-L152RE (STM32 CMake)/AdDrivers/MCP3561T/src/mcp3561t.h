/**
 * @file mcp3561t.h
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef MCP3561T_H
#define MCP3561T_H

/*
What do i want from this driver?
I want it to: 
    +   initialize
    +   read out
*/


// --- INCLUDES ---

#include "mcp3561t_regs.h"
#include "stm32l152xe.h"
#include "stm32l1xx_hal.h"

#include <stdint.h>
#include <stdbool.h>

// --- DEFINES ---

// --- ENUMS ---

typedef enum 
{
    MCP_OK = 0,
    MCP_INVALID_ARG_ERR,
    MCP_RUNTIME_ERR,
    MCP_NOT_INITIALIZED,
    MCP_ALREADY_INITED
    // ....
} MCP3561T_Status_t;


// --- STRUCTS ---

typedef struct
{
    uint8_t dev_id;
    uint8_t config0;
    uint8_t config1;
    uint8_t config2;
    uint8_t config3;
    uint8_t irq;
    uint8_t mux;
    uint32_t scan;
    uint32_t timer;
    uint32_t offsetcal;
    uint32_t gaincal;
    // uint8_t lock;
    // uint16_t crccfg;
} MCP3561T_Config_t;

typedef struct
{
    SPI_HandleTypeDef * hspi;
    GPIO_TypeDef *      cs_port;
    uint16_t            cs_pin;
    MCP3561T_Config_t   config;
    bool                init;
} MCP3561T_Handle_t;


// --- FUNCTIONS ---

// mcp3561t_init.c
MCP3561T_Status_t mcp3561t_Init(MCP3561T_Handle_t * h, SPI_HandleTypeDef * hspi, GPIO_TypeDef * port, uint16_t pin);
MCP3561T_Status_t mcp3561t_SaveConfig(MCP3561T_Handle_t * h, MCP3561T_Config_t * cfg);
MCP3561T_Status_t mcp3561t_Deinit(MCP3561T_Handle_t * h);

// mcp3561t_runtime.c
MCP3561T_Status_t mcp3561t_DeviceReset(MCP3561T_Handle_t * h, uint32_t timeout_ms);
MCP3561T_Status_t mcp3561t_WriteConfig(MCP3561T_Handle_t * h, uint32_t timeout_ms);
MCP3561T_Status_t mcp3561t_ReadConfig(MCP3561T_Handle_t * h, MCP3561T_Config_t * cfg, uint32_t timeout_ms);
MCP3561T_Status_t mcp3561t_ReadAdcRaw(MCP3561T_Handle_t * h, int32_t * raw, uint32_t timeout_ms);


#endif // MCP3561T_H
