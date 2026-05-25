/**
 * @file app_init.c
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "app.h"
#include "comms.h"
#include "config_store.h"
#include "load_output.h"
#include "logger.h"
#include "logger_sd.h"
#include "lcd_ui.h"
#include "mcp3561t.h"
#include "measurement.h"
#include "regulator.h"
#include "safety.h"
#include "app_state.h"
#include "temp_sensor.h"
#include "main.h"
#include "spi.h"
#include "usart.h"

static const char * TAG = "APP_INIT";
static volatile uint8_t s_tick_50ms;


APP_InitStatus_t
app_Init()
{
    AppState_InitDefaults();
    (void)ConfigStore_LoadPartial();

    // first init logging
    if ( LOG_OK != log_Init(&huart2) )
    {
        return APP_INIT_RESTART;
    }

    log_Write(INFO, TAG, "LOG module initialized");
    (void)log_sd_Init();
    log_Write(INFO, TAG, "Set DAC output to zero");

    if ( LoadOutput_Init(&hspi2, CS_3_GPIO_Port, CS_3_Pin) != HAL_OK )
    {
        log_Write(ERR, TAG, "DAC init failed");
        return APP_INIT_RESTART;
    }
    TempSensor_Init();
    LcdUi_Init();
    Safety_Init();

    log_Write(INFO, TAG, "Init ADCs");

    // first adc init
    MCP3561T_Status_t adc_res;
    adc_res = MEAS_CurrentInit();
    if ( MCP_OK != adc_res )
    {
        log_Write(ERR, TAG, "Current ADC init failed with error (%d)", adc_res);
        return APP_INIT_RESTART;
    }

    adc_res = MEAS_VoltageInit();
    if ( MCP_OK != adc_res )
    {
        log_Write(ERR, TAG, "Voltage ADC init failed with error (%d)", adc_res);
        return APP_INIT_RESTART;
    }

    log_Write(INFO, TAG, "All modules and drivers initialized");
    return APP_INIT_DONE;
}


void
app_Notify50msFromIsr(void)
{
    s_tick_50ms = 1U;
}


void
app_Process(void)
{
    if ( s_tick_50ms == 0U )
    {
        return;
    }

    s_tick_50ms = 0U;
    COMMS_Process();
    (void)MEAS_Poll();
    TempSensor_Process();
    Safety_Process();
    (void)Regulator_Run50ms();
    log_sd_Process50ms();
    LcdUi_Process();
}
