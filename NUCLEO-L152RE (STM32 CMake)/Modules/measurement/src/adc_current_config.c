/**
 * @file adc_current_config.c
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "mcp3561t.h"

MCP3561T_Config_t 
adc_current_config(void)
{
    return (MCP3561T_Config_t) {
        .dev_id = 1u,

        .config0 = (uint8_t)(
            (CLK_SEL_INT_CLK_OUTPUT_DIS << MCP3561T_CONFIG0_CLK_SEL_POS) |   // 0b10 << 4
            (CS_SEL_NO_CURRENT_SOURCE   << MCP3561T_CONFIG0_CS_SEL_POS)  |   // 0b00 << 2
            (ADC_MODE_CONVERSION        << MCP3561T_CONFIG0_ADC_MODE_POS)     // 0b11
        ), // = 0x23 (poprawka vs stare 0x63)

        .config1 = (uint8_t)(
            (PRE_AMCLK_MCLK_DIV_1 << MCP3561T_CONFIG1_PRE_POS) |              // 0b00 << 6
            (OSR_98304            << MCP3561T_CONFIG1_OSR_POS)                // 0b1111 << 2
        ), // = 0x3C

        .config2 = (uint8_t)(
            (BOOST_CH_CUR_X1        << MCP3561T_CONFIG2_BOOST_POS) |          // 0b10 << 6
            (GAIN_X1                << MCP3561T_CONFIG2_GAIN_POS)  |          // 0b001 << 3
            (AZ_MUX_AUTO_ZEROING_EN << MCP3561T_CONFIG2_AZ_MUX_POS)           // 1 << 2
        ), // = 0x8C (poprawka vs stare 0x8B)

        .config3 = (uint8_t)(
            (CONV_MODE_CONTINOUS << MCP3561T_CONFIG3_CONV_MODE_POS) |         // 0b11 << 6
            (DATA_FORMAT_24_BIT  << MCP3561T_CONFIG3_DATA_FORMAT_POS) |
            (CRC_FORMAT_16_BIT   << MCP3561T_CONFIG3_CRC_FORMAT_POS)  |
            (EN_OFFCAL_EN        << MCP3561T_CONFIG3_EN_OFFCAL_POS)   |
            (EN_GAINCAL_DIS      << MCP3561T_CONFIG3_EN_GAINCAL_POS)
        ), // = 0xC2

        .irq = (uint8_t)(
            (IRQ_MODE_LH_IRQ  << MCP3561T_IRQ_IRQ_MODE_POS) |
            (EN_FASTCMD_EN    << MCP3561T_IRQ_FASTCMD_POS)  |
            (EN_STP_DIS       << MCP3561T_IRQ_EN_STP_POS)
        ), // = 0x06

        .mux = (uint8_t)(
            (MUX_CH0 << MCP3561T_MUX_MUX_VIN_P_POS) |
            (MUX_CH1 << MCP3561T_MUX_MUX_VIN_N_POS)
        ), // = 0x01

        .scan      = 0u,
        .timer     = 0u,
        .offsetcal = 0u,
        .gaincal   = 0u
    };
}