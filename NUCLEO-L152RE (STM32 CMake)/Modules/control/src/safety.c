#include "safety.h"
#include "app_state.h"
#include "load_output.h"
#include "measurement.h"
#include "stm32l1xx_hal.h"

#define SAFETY_UNDERVOLTAGE_LIMIT 11.2f
#define SAFETY_CHARGE_AH_LIMIT 250.0f
#define SAFETY_TEMP_LIMIT 170.0f

static uint8_t s_prev_onoff;
static uint32_t s_start_tick;
static float s_prev_temp_1;
static SafetyReason_t s_last_reason;

static void stop_regulation(SafetyReason_t reason)
{
    s_last_reason = reason;
    AppState_SetOnOff(0U);
    (void)LoadOutput_SetZero();
}

void Safety_Init(void)
{
    s_prev_onoff = AppState_GetOnOff();
    s_start_tick = HAL_GetTick();
    s_prev_temp_1 = AppState_GetFloat(APP_OFFSET_TEMP_1);
    s_last_reason = SAFETY_REASON_NONE;
}

void Safety_Process(void)
{
    uint8_t onoff = AppState_GetOnOff();

    if ( s_prev_onoff == 0U && onoff == 1U )
    {
        s_start_tick = HAL_GetTick();
        s_last_reason = SAFETY_REASON_NONE;
    }

    if ( onoff != 1U )
    {
        s_prev_onoff = 0U;
        s_prev_temp_1 = AppState_GetFloat(APP_OFFSET_TEMP_1);
        return;
    }

    s_prev_onoff = 1U;

    float time_stop = AppState_GetFloat(APP_OFFSET_TIME_STOP);
    float minutes_elapsed = (float)(HAL_GetTick() - s_start_tick) / (1000.0f * 60.0f);
    float minutes_left = time_stop - minutes_elapsed;

    if ( minutes_left < 0.0f )
    {
        stop_regulation(SAFETY_REASON_TIME_LIMIT);
        AppState_SetLogging(0U);
        return;
    }

    if ( AppState_GetBatteryVoltage() <= SAFETY_UNDERVOLTAGE_LIMIT )
    {
        stop_regulation(SAFETY_REASON_UNDERVOLTAGE);
        return;
    }

    if ( (MEAS_GetChargeCoulombs() / 3600.0f) > SAFETY_CHARGE_AH_LIMIT )
    {
        stop_regulation(SAFETY_REASON_CHARGE_LIMIT);
        return;
    }

    float temp_1 = AppState_GetFloat(APP_OFFSET_TEMP_1);

    if ( temp_1 >= SAFETY_TEMP_LIMIT && s_prev_temp_1 >= SAFETY_TEMP_LIMIT )
    {
        stop_regulation(SAFETY_REASON_OVERTEMPERATURE);
        return;
    }

    s_prev_temp_1 = temp_1;
}

SafetyReason_t Safety_GetLastReason(void)
{
    return s_last_reason;
}
