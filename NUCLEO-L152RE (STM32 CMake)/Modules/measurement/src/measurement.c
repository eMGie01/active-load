/**
 * @file measurement.c
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "measurement.h"
#include "app_state.h"
#include "mcp3561t.h"
#include "logger.h"
#include "main.h"
#include "spi.h"

#define CS_VOLTAGE_PIN CS_1_Pin
#define CS_VOLTAGE_PORT CS_1_GPIO_Port
#define CS_CURRENT_PIN CS_2_Pin
#define CS_CURRENT_PORT CS_2_GPIO_Port

#define MEAS_TIMEOUT_MS 100U

static MCP3561T_Handle_t s_current_adc;
static MCP3561T_Handle_t s_voltage_adc;
static float s_qlmb;

extern MCP3561T_Config_t adc_current_config(void);
extern MCP3561T_Config_t adc_voltage_config(void);

static float current_setpoint_equivalent(void)
{
    float setpoint = AppState_GetSetpoint();
    float voltage = AppState_GetBatteryVoltage();
    uint8_t mode = AppState_GetMode();

    if ( mode == (uint8_t)'P' && voltage != 0.0f )
    {
        return setpoint / voltage;
    }

    if ( mode == (uint8_t)'R' && setpoint != 0.0f )
    {
        return voltage / setpoint;
    }

    return setpoint;
}

static float current_from_raw(int32_t raw)
{
    float setpoint = current_setpoint_equivalent();

    if ( setpoint >= 0.0f && setpoint <= 5.0f )
    {
        return ((float)raw * (4.74f / 294092.3333f)) + (0.17f - 26377.6667f * (4.74f / 294092.3333f));
    }

    if ( setpoint > 5.0f && setpoint <= 10.0f )
    {
        return ((float)raw * (5.086667f / 418925.0f)) + (5.06667f - 333558.333f * (5.086667f / 418925.0f));
    }

    if ( setpoint > 10.0f && setpoint <= 15.0f )
    {
        return ((float)raw * (3.44f / 461250.333f)) + (11.04f - 855380.333f * (3.44f / 461250.333f));
    }

    return 0.0f;
}

static float voltage_from_raw(int32_t raw, float current)
{
    static const float a = 1.4978f / 2913086.0f;
    static const float b = 11.506f - (1838100.0f * a);
    float voltage = ((float)raw * a) + b;

    if ( AppState_GetOnOff() != 0U )
    {
        voltage += current * AppState_GetFloat(APP_OFFSET_WIRE_RESISTANCE);
    }

    return voltage;
}

static void update_range_relays(float target_current)
{
    if ( target_current <= 5.0f )
    {
        HAL_GPIO_WritePin(RES_1_GPIO_Port, RES_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RES_2_GPIO_Port, RES_2_Pin, GPIO_PIN_RESET);
    }
    else if ( target_current <= 11.0f )
    {
        HAL_GPIO_WritePin(RES_1_GPIO_Port, RES_1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RES_2_GPIO_Port, RES_2_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(RES_1_GPIO_Port, RES_1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RES_2_GPIO_Port, RES_2_Pin, GPIO_PIN_SET);
    }
}


MCP3561T_Status_t
MEAS_CurrentInit()
{
    MCP3561T_Status_t res;
    /* Legacy mapping: current ADC is on CS_2, voltage ADC is on CS_1. */
    res = mcp3561t_Init(&s_current_adc, &hspi2, CS_CURRENT_PORT, CS_CURRENT_PIN);
    if ( MCP_OK != res ) return res;
 
    MCP3561T_Config_t adc_current_cfg = adc_current_config();
    res = mcp3561t_SaveConfig(&s_current_adc, &adc_current_cfg);
    if ( MCP_OK != res) return res;

    res = mcp3561t_WriteConfig(&s_current_adc, 100);
    if (res != MCP_OK) return res;

    return MCP_OK;
}

MCP3561T_Status_t
MEAS_VoltageInit()
{
    MCP3561T_Status_t res;
    /* SD remains on SPI3; both ADCs are intentionally on SPI2. */
    res = mcp3561t_Init(&s_voltage_adc, &hspi2, CS_VOLTAGE_PORT, CS_VOLTAGE_PIN);
    if ( MCP_OK != res ) return res;
 
    MCP3561T_Config_t adc_voltage_cfg = adc_voltage_config();
    res = mcp3561t_SaveConfig(&s_voltage_adc, &adc_voltage_cfg);
    if ( MCP_OK != res) return res;

    res = mcp3561t_WriteConfig(&s_voltage_adc, 100);
    if (res != MCP_OK) return res;

    return MCP_OK;
}

MCP3561T_Status_t
MEAS_ReadCurrent(float *current)
{
    if ( current == NULL )
    {
        return MCP_INVALID_ARG_ERR;
    }

    int32_t raw = 0;
    MCP3561T_Status_t res = mcp3561t_ReadAdcRaw(&s_current_adc, &raw, MEAS_TIMEOUT_MS);

    if ( res != MCP_OK )
    {
        return res;
    }

    *current = current_from_raw(raw);

    if ( AppState_GetOnOff() != 1U )
    {
        *current = 0.0f;
    }

    return MCP_OK;
}

MCP3561T_Status_t
MEAS_ReadVoltage(float *voltage)
{
    if ( voltage == NULL )
    {
        return MCP_INVALID_ARG_ERR;
    }

    int32_t raw = 0;
    MCP3561T_Status_t res = mcp3561t_ReadAdcRaw(&s_voltage_adc, &raw, MEAS_TIMEOUT_MS);

    if ( res != MCP_OK )
    {
        return res;
    }

    *voltage = voltage_from_raw(raw, AppState_GetCurrent());
    return MCP_OK;
}

MCP3561T_Status_t
MEAS_Poll(void)
{
    float current = 0.0f;
    float voltage = 0.0f;
    MCP3561T_Status_t res = MEAS_ReadCurrent(&current);

    if ( res != MCP_OK )
    {
        return res;
    }

    AppState_SetCurrentValue(current);
    update_range_relays(current_setpoint_equivalent());

    res = MEAS_ReadVoltage(&voltage);
    if ( res != MCP_OK )
    {
        return res;
    }

    AppState_SetBatteryVoltageValue(voltage);
    AppState_UpdateParameter();

    if ( AppState_GetOnOff() == 1U )
    {
        s_qlmb += current * 0.05f;
    }
    else
    {
        s_qlmb = 0.0f;
    }

    AppState_UpdateCrc();
    return MCP_OK;
}

float
MEAS_GetChargeCoulombs(void)
{
    return s_qlmb;
}
