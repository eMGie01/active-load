/**
 * @file mcp3561t_init.c
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "mcp3561t.h"
#include "stm32l1xx_hal_def.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_spi.h"
#include <string.h>


MCP3561T_Status_t 
mcp3561t_Init(MCP3561T_Handle_t * h, SPI_HandleTypeDef * hspi, GPIO_TypeDef * port, uint16_t pin)
{
    if ( !h || !hspi || !port )
    {
        return MCP_INVALID_ARG_ERR;
    }

    if ( h->init )
    {
        return MCP_ALREADY_INITED;
    }

    h->hspi = hspi;
    h->cs_port = port;
    h->cs_pin = pin;

    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);
    h->init = true;

    return MCP_OK;
}


MCP3561T_Status_t
mcp3561t_SaveConfig(MCP3561T_Handle_t *h, MCP3561T_Config_t *cfg)
{
    if ( !h || !cfg || !h->hspi || !h->cs_port || cfg->dev_id > 3u )
    {
        return MCP_INVALID_ARG_ERR;
    }

    if ( !h->init )
    {
        return MCP_NOT_INITIALIZED;
    }

    h->config = *cfg;

    h->config.config0 &= (MCP3561T_CONFIG0_CLK_SEL_MSK |
                          MCP3561T_CONFIG0_CS_SEL_MSK  |
                          MCP3561T_CONFIG0_ADC_MODE_MSK);

    h->config.config1 &= (MCP3561T_CONFIG1_PRE_MSK |
                          MCP3561T_CONFIG1_OSR_MSK);

    h->config.config2   &= (MCP3561T_CONFIG2_BOOST_MSK |
                            MCP3561T_CONFIG2_GAIN_MSK  |
                            MCP3561T_CONFIG2_AZ_MUX_MSK);

    h->config.config3   &= (MCP3561T_CONFIG3_CONV_MODE_MSK   |
                            MCP3561T_CONFIG3_DATA_FORMAT_MSK |
                            MCP3561T_CONFIG3_CRC_FORMAT_MSK  |
                            MCP3561T_CONFIG3_EN_CRCCOM_MSK   |
                            MCP3561T_CONFIG3_EN_OFFCAL_MSK   |
                            MCP3561T_CONFIG3_EN_GAINCAL_MSK);

    h->config.irq       &= (MCP3561T_IRQ_IRQ_MODE_MSK |
                            MCP3561T_IRQ_FASTCMD_MSK  |
                            MCP3561T_IRQ_EN_STP_MSK);

    h->config.mux       &= (MCP3561T_MUX_MUX_VIN_P_MSK |
                            MCP3561T_MUX_MUX_VIN_N_MSK);

    h->config.scan      &= MCP3561T_SCAN_DLY_MSK | MCP3561T_SCAN_CHANNEL_SEL_MSK;
    h->config.timer     &= MCP3561T_TIMER_MSK;
    h->config.offsetcal &= MCP3561T_OFFSETCAL_MSK;
    h->config.gaincal   &= MCP3561T_GAINCAL_MSK;
    
    return MCP_OK;
}


MCP3561T_Status_t
mcp3561t_Deinit(MCP3561T_Handle_t *h)
{
    if ( !h )
    {
        return MCP_INVALID_ARG_ERR;
    }

    if ( h->cs_port )
    {
        HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);
    }

    memset(h, 0, sizeof(*h));
    return MCP_OK;
}
