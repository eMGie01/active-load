/**
 * @file mcp3561t_regs.h
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef MCP3561T_REGS_H
#define MCP3561T_REGS_H


// --- DEFINES ---

#define MCP3561T_REG_ADCDATA    0x0u
#define MCP3561T_REG_CONFIG0    0x1u
#define MCP3561T_REG_CONFIG1    0x2u
#define MCP3561T_REG_CONFIG2    0x3u
#define MCP3561T_REG_CONFIG3    0x4u
#define MCP3561T_REG_IRQ        0x5u
#define MCP3561T_REG_MUX        0x6u
#define MCP3561T_REG_SCAN       0x7u
#define MCP3561T_REG_TIMER      0x8u
#define MCP3561T_REG_OFFSETCAL  0x9u
#define MCP3561T_REG_GAINCAL    0xAu
#define MCP3561T_REG_LOCK       0xDu
#define MCP3561T_REG_CRCCFG     0xFu

// config 0
#define MCP3561T_CONFIG0_CLK_SEL_POS     4u
#define MCP3561T_CONFIG0_CS_SEL_POS      2u
#define MCP3561T_CONFIG0_ADC_MODE_POS    0u
#define MCP3561T_CONFIG0_CLK_SEL_MSK    (0x3u << MCP3561T_CONFIG0_CLK_SEL_POS)
#define MCP3561T_CONFIG0_CS_SEL_MSK     (0x3u << MCP3561T_CONFIG0_CS_SEL_POS)
#define MCP3561T_CONFIG0_ADC_MODE_MSK   (0x3u << MCP3561T_CONFIG0_ADC_MODE_POS)

// config 1
#define MCP3561T_CONFIG1_PRE_POS    6u
#define MCP3561T_CONFIG1_OSR_POS    2u
#define MCP3561T_CONFIG1_PRE_MSK    (0x3u << MCP3561T_CONFIG1_PRE_POS)
#define MCP3561T_CONFIG1_OSR_MSK    (0xFu << MCP3561T_CONFIG1_OSR_POS)

// config 2
#define MCP3561T_CONFIG2_BOOST_POS  6u
#define MCP3561T_CONFIG2_GAIN_POS   3u
#define MCP3561T_CONFIG2_AZ_MUX_POS 2u
#define MCP3561T_CONFIG2_BOOST_MSK  (0x3u << MCP3561T_CONFIG2_BOOST_POS)
#define MCP3561T_CONFIG2_GAIN_MSK   (0x7u << MCP3561T_CONFIG2_GAIN_POS)
#define MCP3561T_CONFIG2_AZ_MUX_MSK (0x1u << MCP3561T_CONFIG2_AZ_MUX_POS)

// config 3
#define MCP3561T_CONFIG3_CONV_MODE_POS      6u
#define MCP3561T_CONFIG3_DATA_FORMAT_POS    4u
#define MCP3561T_CONFIG3_CRC_FORMAT_POS     3u
#define MCP3561T_CONFIG3_EN_CRCCOM_POS      2u
#define MCP3561T_CONFIG3_EN_OFFCAL_POS      1u
#define MCP3561T_CONFIG3_EN_GAINCAL_POS     0u
#define MCP3561T_CONFIG3_CONV_MODE_MSK      (0x3u << MCP3561T_CONFIG3_CONV_MODE_POS)
#define MCP3561T_CONFIG3_DATA_FORMAT_MSK    (0x3u << MCP3561T_CONFIG3_DATA_FORMAT_POS)
#define MCP3561T_CONFIG3_CRC_FORMAT_MSK     (0x1u << MCP3561T_CONFIG3_CRC_FORMAT_POS)
#define MCP3561T_CONFIG3_EN_CRCCOM_MSK      (0x1u << MCP3561T_CONFIG3_EN_CRCCOM_POS)
#define MCP3561T_CONFIG3_EN_OFFCAL_MSK      (0x1u << MCP3561T_CONFIG3_EN_OFFCAL_POS)
#define MCP3561T_CONFIG3_EN_GAINCAL_MSK     (0x1u << MCP3561T_CONFIG3_EN_GAINCAL_POS)

// irq
#define MCP3561T_IRQ_DR_STATUS_POS      6u
#define MCP3561T_IRQ_CRCCFG_STATUS_POS  5u
#define MCP3561T_IRQ_POR_STATUS_POS     4u
#define MCP3561T_IRQ_IRQ_MODE_POS       2u
#define MCP3561T_IRQ_FASTCMD_POS        1u
#define MCP3561T_IRQ_EN_STP_POS         0u
#define MCP3561T_IRQ_DR_STATUS_MSK      (0x1u << MCP3561T_IRQ_DR_STATUS_POS)
#define MCP3561T_IRQ_CRCCFG_STATUS_MSK  (0x1u << MCP3561T_IRQ_CRCCFG_STATUS_POS)
#define MCP3561T_IRQ_POR_STATUS_MSK     (0x1u << MCP3561T_IRQ_POR_STATUS_POS)
#define MCP3561T_IRQ_IRQ_MODE_MSK       (0x3u << MCP3561T_IRQ_IRQ_MODE_POS)
#define MCP3561T_IRQ_FASTCMD_MSK        (0x1u << MCP3561T_IRQ_FASTCMD_POS)
#define MCP3561T_IRQ_EN_STP_MSK         (0x1u << MCP3561T_IRQ_EN_STP_POS)

// mux
#define MCP3561T_MUX_MUX_VIN_P_POS      4u
#define MCP3561T_MUX_MUX_VIN_N_POS      0u
#define MCP3561T_MUX_MUX_VIN_P_MSK      (0xFu << MCP3561T_MUX_MUX_VIN_P_POS)
#define MCP3561T_MUX_MUX_VIN_N_MSK      (0xFu << MCP3561T_MUX_MUX_VIN_N_POS)

// scan
#define MCP3561T_SCAN_DLY_POS           21u
#define MCP3561T_SCAN_CHANNEL_SEL_POS   0u
#define MCP3561T_SCAN_DLY_MSK           (0x7u << MCP3561T_SCAN_DLY_POS)
#define MCP3561T_SCAN_CHANNEL_SEL_MSK   (0xFFFFu << MCP3561T_SCAN_CHANNEL_SEL_POS)

// timer
#define MCP3561T_TIMER_POS    0u
#define MCP3561T_TIMER_MSK    (0xFFFFFFu << MCP3561T_TIMER_POS)

// offset 
#define MCP3561T_OFFSETCAL_POS    0u
#define MCP3561T_OFFSETCAL_MSK    (0xFFFFFFu << MCP3561T_OFFSETCAL_POS)

// gaincal
#define MCP3561T_GAINCAL_POS    0u
#define MCP3561T_GAINCAL_MSK    (0xFFFFFFu << MCP3561T_GAINCAL_POS)

// lock
#define MCP3561T_LOCK_POS    0u
#define MCP3561T_LOCK_MSK    (0xFu << MCP3561T_LOCK_POS)

// crccfg
#define MCP3561T_CRCCFG_POS    0u
#define MCP3561T_CRCCFG_MSK    (0xFFu << MCP3561T_CRCCFG_POS)


// --- ENUMS ---

// config 0 enums
#define ADC_MODE_DEF ADC_MODE_SHUTDOWN_DEF
typedef enum
{
    ADC_MODE_SHUTDOWN = 0,
    ADC_MODE_SHUTDOWN_1 = 1,
    ADC_MODE_STANDBY = 2,
    ADC_MODE_CONVERSION = 3
} mcp3561t_adc_mode_t;

#define CS_SEL_DEF CS_SEL_NO_CURRENT_SOURCE
typedef enum
{
    CS_SEL_NO_CURRENT_SOURCE = 0,
    CS_SEL_900_NA = 1,
    CS_SEL_3700_NA = 2,
    CS_SEL_15_UA = 3
} mcp3561t_cs_sel_t;

#define CLK_SEL_DEF CLK_SEL_EXT_DIG_CLK_DEF
typedef enum
{
    CLK_SEL_EXT_DIG_CLK = 0,
    CLK_SEL_EXT_DIG_CLK_1 = 1,
    CLK_SEL_INT_CLK_OUTPUT_DIS = 2,
    CLK_SEL_INT_CLK_OUTPUT_EN = 3
} mcp3561t_clk_sel_t;

typedef enum
{
    CONFIG0_FULL_SHUTDOWN = 0,
    CONFIG0_NO_SHUTDOWN = 1
} mcp3561t_shdn_mode_t;


// config1 enums
#define OSR_DEF OSR_256
typedef enum
{
    OSR_32 = 0,
    OSR_64,
    OSR_128,
    OSR_256,
    OSR_512,
    OSR_1024,
    OSR_2048,
    OSR_4096,
    OSR_8192,
    OSR_16384,
    OSR_20480,
    OSR_24576,
    OSR_40960,
    OSR_49152,
    OSR_81920,
    OSR_98304
} mcp3561t_osr_t;

#define PRE_DEF PRE_AMCLK_MCLK_DIV_1
typedef enum
{
    PRE_AMCLK_MCLK_DIV_1 = 0,
    PRE_AMCLK_MCLK_DIV_2,
    PRE_AMCLK_MCLK_DIV_4,
    PRE_AMCLK_MCLK_DIV_8
} mcp3561t_pre_t;


// config2 enums
#define AZ_MUX_DEF AZ_MUX_AUTO_ZEROING_DISABLED
typedef enum
{
    AZ_MUX_AUTO_ZEROING_DIS = 0,
    AZ_MUX_AUTO_ZEROING_EN
} mcp3561t_az_mux_t;

#define GAIN_DEF GAIN_X1
typedef enum
{
    GAIN_X0_33 = 0,
    GAIN_X1 = 1,
    GAIN_X2,
    GAIN_X4,
    GAIN_X8,
    GAIN_X16,
    GAIN_X32,
    GAIN_X64
} mcp3561t_gain_t;

#define BOOST_DEF BOOST_CH_CUR_X1
typedef enum
{
    BOOST_CH_CUR_X0_5 = 0,
    BOOST_CH_CUR_X0_66,
    BOOST_CH_CUR_X1,
    BOOST_CH_CUR_X2
} mcp3561t_boost_t;


// config3 enums
#define EN_GAINCAL_DEF EN_GAINCAL_DIS
typedef enum
{
    EN_GAINCAL_DIS = 0,
    EN_GAINCAL_EN = 1
} mcp3561t_en_gaincal_t;

#define EN_OFFCAL_DEF EN_OFFCAL_DIS
typedef enum
{
    EN_OFFCAL_DIS = 0,
    EN_OFFCAL_EN = 1
} mcp3561t_en_offcal_t;

#define CRC_FORMAT_DEF CRC_FORMAT_16_BIT
typedef enum
{
    CRC_FORMAT_16_BIT = 0,
    CRC_FORMAT_32_BIT = 1
} mcp3561t_crc_format_t;

#define DATA_FORMAT_DEF DATA_FORMAT_24_BIT
typedef enum
{
    DATA_FORMAT_24_BIT = 0,             // Default
    DATA_FORMAT_32_BIT_LEFT_J = 1,      // (24-bit data + 8-bit zero), no overrange
    DATA_FORMAT_32_BIT_SGN_EXT = 2,     // (24-bit data + 8-bit SGN), allows overrange
    ADC_FORMAT_32_BIT_WITH_CHID  = 3    //(24-bit data + SGN + Channel ID), allows overrange
} mcp3561t_data_format_t;

#define CONV_MODE_DEF CONV_MODE_ONESHOT_SHDN
typedef enum
{
    CONV_MODE_ONESHOT_SHDN = 0,
    CONV_MODE_ONESHOT_SHDN_1 = 1,
    CONV_MODE_ONESHOT_STDNBY = 2,
    CONV_MODE_CONTINOUS = 3
} mcp3561t_conv_mode_t;

// irq enums
#define EN_STP_DEF EN_STP_EN
typedef enum
{
    EN_STP_DIS = 0,
    EN_STP_EN = 1
} mcp3561t_en_stp_t;

#define EN_FASTCMD_DEF EN_FASTCMD_EN
typedef enum
{
    EN_FASTCMD_DIS = 0,
    EN_FASTCMD_EN = 1
} mcp3561t_en_fastcmd_t;

#define IRQ_MODE_DEF IRQ_MODE_HZ_IRQ
typedef enum
{
    IRQ_MODE_HZ_IRQ = 0,
    IRQ_MODE_LH_IRQ = 1,
    IRQ_MODE_HZ_MDAT = 2,
    IRQ_MODE_LH_MDAT = 3
} mcp3561_irq_mode_t;

#define POR_STATUS_DEF POR_STATUS_NOT_OCCURED
typedef enum
{
    POR_STATUS_OCCURED = 0,
    POR_STATUS_NOT_OCCURED = 1
} mcp3561t_por_status_t;

#define CRCCFG_STATUS_DEF CRCCFG_STATUS_NOT_OCCURED
typedef enum
{
    CRCCFG_STATUS_OCCURED = 0,
    CRCCFG_STATUS_NOT_OCCURED = 1
} mcp3561t_crccfg_status_t;

#define DR_STATUS_DEF DR_STATUS_NOT_RDY
typedef enum
{
    DR_STATUS_RDY = 0,
    DR_STATUS_NOT_RDY = 1
} mcp3561t_dr_status_t;


// mux enums
#define MUX_POS_DEF MUX_CH0
#define MUX_POS_NEG MUX_CH1
typedef enum
{
    MUX_CH0 = 0,
    MUX_CH1,
    MUX_CH2,
    MUX_CH3,
    MUX_CH4,
    MUX_CH5,
    MUX_CH6,
    MUX_CH7,
    MUX_AGND,
    MUX_AVDD,
    MUX_REFIN_POS = 11,
    MUX_REFIN_NEG,
    MUX_TEMP_DIODE_P,
    MUX_TEMP_DIODE_M,
    MUX_INTERNAL_VCM
} mcp3561_mux_t;


// scan enums
#define DLY_DEF DLY_DIS
typedef enum
{
    DLY_DIS = 0,
    DLY_DMCLK_X8,
    DLY_DMCLK_X16,
    DLY_DMCLK_X32,
    DLY_DMCLK_X64,
    DLY_DMCLK_X128,
    DLY_DMCLK_X256,
    DLY_DMCLK_X512
} mcp3561t_dly_t;

typedef enum
{
    CH_SEL_SINGLE_CH0 = 0,
    CH_SEL_SINGLE_CH1,
    CH_SEL_SINGLE_CH2,
    CH_SEL_SINGLE_CH3,
    CH_SEL_SINGLE_CH4,
    CH_SEL_SINGLE_CH5,
    CH_SEL_SINGLE_CH6,
    CH_SEL_SINGLE_CH7,
    CH_SEL_DIFF_A,
    CH_SEL_DIFF_B,
    CH_SEL_DIFF_C,
    CH_SEL_DIFF_D,
    CH_SEL_TEMP,
    CH_SEL_AVDD,
    CH_SEL_VCM,
    CH_SEL_OFFSET
} mcp3561t_ch_sel_t;


// --- END ---

#endif // MCP3561T_REGS_H
