/**
 * @file adc_voltage_config.c
 */
#include "mcp3561t.h"

MCP3561T_Config_t
adc_voltage_config(void)
{
    return (MCP3561T_Config_t){
        .dev_id = 1u, 

        .config0 = (uint8_t)(
            (CLK_SEL_INT_CLK_OUTPUT_DIS << MCP3561T_CONFIG0_CLK_SEL_POS) |
            (CS_SEL_NO_CURRENT_SOURCE   << MCP3561T_CONFIG0_CS_SEL_POS)  |
            (ADC_MODE_CONVERSION        << MCP3561T_CONFIG0_ADC_MODE_POS)
        ), // 0x23 (zamiast starego 0x63)

        .config1 = (uint8_t)(
            (PRE_AMCLK_MCLK_DIV_1 << MCP3561T_CONFIG1_PRE_POS) |
            (OSR_98304            << MCP3561T_CONFIG1_OSR_POS)
        ), // 0x3C

        .config2 = (uint8_t)(
            (BOOST_CH_CUR_X1        << MCP3561T_CONFIG2_BOOST_POS) |
            (GAIN_X1                << MCP3561T_CONFIG2_GAIN_POS)  |
            (AZ_MUX_AUTO_ZEROING_EN << MCP3561T_CONFIG2_AZ_MUX_POS)
        ), // 0x8C (stare było 0x8B)

        .config3 = (uint8_t)(
            (CONV_MODE_CONTINOUS << MCP3561T_CONFIG3_CONV_MODE_POS) |
            (DATA_FORMAT_24_BIT  << MCP3561T_CONFIG3_DATA_FORMAT_POS) |
            (CRC_FORMAT_16_BIT   << MCP3561T_CONFIG3_CRC_FORMAT_POS)
        ), // 0xC0 (jak w starym)

        .irq = (uint8_t)(
            (IRQ_MODE_LH_IRQ << MCP3561T_IRQ_IRQ_MODE_POS) |
            (EN_FASTCMD_EN   << MCP3561T_IRQ_FASTCMD_POS)  |
            (EN_STP_DIS      << MCP3561T_IRQ_EN_STP_POS)
        ), // 0x06

        .mux = (uint8_t)(
            (MUX_CH0  << MCP3561T_MUX_MUX_VIN_P_POS) |
            (MUX_AGND << MCP3561T_MUX_MUX_VIN_N_POS)
        ), // 0x08 (jak w starym)

        .scan = 0u,
        .timer = 0u,
        .offsetcal = 0u,
        .gaincal = 0u
    };
}
