/**
 * @file mcp3561t_runtime.c
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "mcp3561t.h"
#include "stm32l1xx_hal_def.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_spi.h"


#define FAST_COMMAND        0b00
#define STATIC_READ         0b01
#define INCREMENTAL_WRITE   0b10
#define INCREMENTAL_READ    0b11

#define FAST_ADC_START  0xA
#define FAST_ADC_STNBY  0xB
#define FAST_ADC_SHTDN  0xC
#define FAST_FUL_SHTDN  0xD
#define FAST_DEV_RESET  0xE


static bool
write_reg(const MCP3561T_Handle_t * h, uint8_t reg_addr, uint8_t * data, uint16_t len, uint32_t timeout_ms)
{
    if ( !h || !h->hspi || !h->cs_port || !h->init || data == NULL || len == 0)
    {
        return false;
    }
    uint8_t cmnd_byte = ((h->config.dev_id & 0x03) << 6) | ((reg_addr & 0x0F) << 2) | INCREMENTAL_WRITE;
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_RESET);
    bool output = (HAL_OK == HAL_SPI_Transmit(h->hspi, &cmnd_byte, 1, timeout_ms)) &&
                  (HAL_OK == HAL_SPI_Transmit(h->hspi, data, len, timeout_ms));
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);
    return output;
}


static bool
read_reg(const MCP3561T_Handle_t * h, uint8_t reg_addr, uint8_t * data, uint16_t len, uint32_t timeout_ms)
{
    if ( !h || !h->hspi || !h->cs_port || !h->init || data == NULL || len == 0)
    {
        return false;
    }
    uint8_t cmnd_byte = ((h->config.dev_id & 0x03) << 6) | ((reg_addr & 0x0F) << 2) | STATIC_READ;
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_RESET);
    bool output = (HAL_OK == HAL_SPI_Transmit(h->hspi, &cmnd_byte, 1, timeout_ms)) && 
                  (HAL_OK == HAL_SPI_Receive(h->hspi, data, len, timeout_ms));
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);
    return output;
}


static bool
fast_cmnd(const MCP3561T_Handle_t * h, uint8_t fast_code, uint32_t timeout_ms)
{
    if ( !h || !h->hspi || !h->cs_port || !h->init )
    {
        return false;
    }
    uint8_t fast_cmnd_byte = ((h->config.dev_id & 0x03) << 6) | ((fast_code & 0x0F) << 2) | FAST_COMMAND;
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_RESET);
    bool output = ( HAL_OK == HAL_SPI_Transmit(h->hspi, &fast_cmnd_byte, 1, timeout_ms) );
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);
    return output;
}


MCP3561T_Status_t
mcp3561t_DeviceReset(MCP3561T_Handle_t * h, uint32_t timeout_ms)
{
    if ( fast_cmnd(h, FAST_DEV_RESET, timeout_ms) )
    {
        return MCP_OK;
    }
    return MCP_RUNTIME_ERR;
}


MCP3561T_Status_t
mcp3561t_WriteConfig(MCP3561T_Handle_t * h, uint32_t timeout_ms)
{
    if ( !h || !h->init || !h->hspi || !h->cs_port )
    {
        return MCP_INVALID_ARG_ERR;
    }

    uint8_t reg = 0;
    uint8_t reg24[3];

    reg = h->config.config0;
    if ( !write_reg(h, MCP3561T_REG_CONFIG0, &reg, 1, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg = h->config.config1;
    if ( !write_reg(h, MCP3561T_REG_CONFIG1, &reg, 1, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg = h->config.config2;
    if ( !write_reg(h, MCP3561T_REG_CONFIG2, &reg, 1, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg = h->config.config3;
    if ( !write_reg(h, MCP3561T_REG_CONFIG3, &reg, 1, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg = h->config.irq;
    if ( !write_reg(h, MCP3561T_REG_IRQ, &reg, 1, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg = h->config.mux;
    if ( !write_reg(h, MCP3561T_REG_MUX, &reg, 1, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg24[0] = (uint8_t)(h->config.scan >> 16);
    reg24[1] = (uint8_t)(h->config.scan >> 8);
    reg24[2] = (uint8_t)(h->config.scan);
    if ( !write_reg(h, MCP3561T_REG_SCAN, reg24, 3, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg24[0] = (uint8_t)(h->config.timer >> 16);
    reg24[1] = (uint8_t)(h->config.timer >> 8);
    reg24[2] = (uint8_t)(h->config.timer);
    if ( !write_reg(h, MCP3561T_REG_TIMER, reg24, 3, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg24[0] = (uint8_t)(h->config.offsetcal >> 16);
    reg24[1] = (uint8_t)(h->config.offsetcal >> 8);
    reg24[2] = (uint8_t)(h->config.offsetcal);
    if ( !write_reg(h, MCP3561T_REG_OFFSETCAL, reg24, 3, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    reg24[0] = (uint8_t)(h->config.gaincal >> 16);
    reg24[1] = (uint8_t)(h->config.gaincal >> 8);
    reg24[2] = (uint8_t)(h->config.gaincal);
    if ( !write_reg(h, MCP3561T_REG_GAINCAL, reg24, 3, timeout_ms) )
    {
        return MCP_RUNTIME_ERR;
    }

    return MCP_OK;
}


MCP3561T_Status_t
mcp3561t_ReadConfig(MCP3561T_Handle_t *h, MCP3561T_Config_t *cfg, uint32_t timeout_ms)
{
    if (!h || !cfg || !h->init || !h->hspi || !h->cs_port) {
        return MCP_INVALID_ARG_ERR;
    }

    uint8_t b = 0;
    uint8_t b24[3] = {0};

    cfg->dev_id = h->config.dev_id; // ID z handle

    if (!read_reg(h, MCP3561T_REG_CONFIG0, &b, 1, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->config0 = b & (MCP3561T_CONFIG0_CLK_SEL_MSK | MCP3561T_CONFIG0_CS_SEL_MSK | MCP3561T_CONFIG0_ADC_MODE_MSK);

    if (!read_reg(h, MCP3561T_REG_CONFIG1, &b, 1, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->config1 = b & (MCP3561T_CONFIG1_PRE_MSK | MCP3561T_CONFIG1_OSR_MSK);

    if (!read_reg(h, MCP3561T_REG_CONFIG2, &b, 1, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->config2 = b & (MCP3561T_CONFIG2_BOOST_MSK | MCP3561T_CONFIG2_GAIN_MSK | MCP3561T_CONFIG2_AZ_MUX_MSK);

    if (!read_reg(h, MCP3561T_REG_CONFIG3, &b, 1, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->config3 = b & (MCP3561T_CONFIG3_CONV_MODE_MSK | MCP3561T_CONFIG3_DATA_FORMAT_MSK |
                        MCP3561T_CONFIG3_CRC_FORMAT_MSK | MCP3561T_CONFIG3_EN_CRCCOM_MSK |
                        MCP3561T_CONFIG3_EN_OFFCAL_MSK | MCP3561T_CONFIG3_EN_GAINCAL_MSK);

    if (!read_reg(h, MCP3561T_REG_IRQ, &b, 1, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->irq = b; // tu masz też bity statusowe DR/CRCCFG/POR

    if (!read_reg(h, MCP3561T_REG_MUX, &b, 1, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->mux = b & (MCP3561T_MUX_MUX_VIN_P_MSK | MCP3561T_MUX_MUX_VIN_N_MSK);

    if (!read_reg(h, MCP3561T_REG_SCAN, b24, 3, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->scan = ((uint32_t)b24[0] << 16) | ((uint32_t)b24[1] << 8) | b24[2];

    if (!read_reg(h, MCP3561T_REG_TIMER, b24, 3, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->timer = ((uint32_t)b24[0] << 16) | ((uint32_t)b24[1] << 8) | b24[2];

    if (!read_reg(h, MCP3561T_REG_OFFSETCAL, b24, 3, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->offsetcal = ((uint32_t)b24[0] << 16) | ((uint32_t)b24[1] << 8) | b24[2];

    if (!read_reg(h, MCP3561T_REG_GAINCAL, b24, 3, timeout_ms)) return MCP_RUNTIME_ERR;
    cfg->gaincal = ((uint32_t)b24[0] << 16) | ((uint32_t)b24[1] << 8) | b24[2];

    return MCP_OK;
}


MCP3561T_Status_t
mcp3561t_ReadAdcRaw(MCP3561T_Handle_t *h, int32_t *raw, uint32_t timeout_ms)
{
    if (!h || !raw || !h->init || !h->hspi || !h->cs_port) {
        return MCP_INVALID_ARG_ERR;
    }

    uint8_t b24[3] = {0};

    if (!read_reg(h, MCP3561T_REG_ADCDATA, b24, 3, timeout_ms)) {
        return MCP_RUNTIME_ERR;
    }

    *raw = ((int32_t)b24[0] << 16) | ((int32_t)b24[1] << 8) | (int32_t)b24[2];
    return MCP_OK;
}
