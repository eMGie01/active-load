#include "regulator.h"
#include "app_state.h"
#include "load_output.h"
#include <stdint.h>

#define REGULATOR_DT_SECONDS 0.05f
#define REGULATOR_CURRENT_MAX 15.0f
#define REGULATOR_DAC_GAIN_CODE 37683.0f
#define REGULATOR_DAC_OFFSET_CODE 27852.0f

static float s_integral;
static float s_prev_error;

static float clamp_float(float value, float min_value, float max_value)
{
    if ( value < min_value )
    {
        return min_value;
    }

    if ( value > max_value )
    {
        return max_value;
    }

    return value;
}

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

float Regulator_Compensate(float current, float temperature)
{
    const float vgs_table[3][16] = {
        {2.100f, 2.266f, 2.351f, 2.420f, 2.480f, 2.534f, 2.585f, 2.632f,
         2.678f, 2.721f, 2.763f, 2.804f, 2.843f, 2.882f, 2.919f, 2.956f},
        {2.002f, 2.223f, 2.321f, 2.400f, 2.466f, 2.528f, 2.585f, 2.638f,
         2.690f, 2.738f, 2.786f, 2.831f, 2.875f, 2.918f, 2.960f, 3.002f},
        {1.900f, 2.176f, 2.286f, 2.372f, 2.448f, 2.517f, 2.580f, 2.639f,
         2.696f, 2.749f, 2.802f, 2.852f, 2.900f, 2.948f, 2.994f, 3.039f}
    };

    current = clamp_float(current, 0.0f, 14.999f);
    temperature = clamp_float(temperature, 25.0f, 175.0f);

    int current_idx = (int)current;
    int temp_l_idx = 0;
    int temp_h_idx = 0;
    float corr_temp = 0.0f;

    if ( temperature < 100.0f )
    {
        temp_l_idx = 0;
        temp_h_idx = 1;
        corr_temp = (temperature - 25.0f) / 75.0f;
    }
    else
    {
        temp_l_idx = 1;
        temp_h_idx = 2;
        corr_temp = (temperature - 100.0f) / 75.0f;
    }

    float frac = current - (float)current_idx;
    float corr_25 = vgs_table[0][current_idx] + frac * (vgs_table[0][current_idx + 1] - vgs_table[0][current_idx]);

    if ( corr_25 < 0.001f )
    {
        return 1.0f;
    }

    float corr_t_low = vgs_table[temp_l_idx][current_idx] + frac * (vgs_table[temp_l_idx][current_idx + 1] - vgs_table[temp_l_idx][current_idx]);
    float corr_t_high = vgs_table[temp_h_idx][current_idx] + frac * (vgs_table[temp_h_idx][current_idx + 1] - vgs_table[temp_h_idx][current_idx]);

    return (corr_t_low + corr_temp * (corr_t_high - corr_t_low)) / corr_25;
}

HAL_StatusTypeDef Regulator_Run50ms(void)
{
    if ( AppState_GetOnOff() == 0U )
    {
        s_integral = 0.0f;
        s_prev_error = 0.0f;
        return LoadOutput_SetZero();
    }

    float setpoint = current_setpoint_equivalent();
    float current = AppState_GetCurrent();
    float temp_1 = AppState_GetFloat(APP_OFFSET_TEMP_1);
    float kp = AppState_GetFloat(APP_OFFSET_KP);
    float ki = AppState_GetFloat(APP_OFFSET_KI);
    float kd = AppState_GetFloat(APP_OFFSET_KD);

    float error = setpoint - current;
    float derivative = (error - s_prev_error) / REGULATOR_DT_SECONDS;
    s_prev_error = error;

    float compensation = Regulator_Compensate(current, temp_1);
    float provisional_output = ((kp * error) + (ki * s_integral) + (kd * derivative)) * compensation;

    if ( provisional_output > 0.0f && provisional_output < REGULATOR_CURRENT_MAX )
    {
        s_integral += error * REGULATOR_DT_SECONDS;
    }

    float output = ((kp * error) + (ki * s_integral) + (kd * derivative)) * compensation;
    output = clamp_float(output, 0.0f, REGULATOR_CURRENT_MAX);

    uint16_t dac_code = (uint16_t)((output / REGULATOR_CURRENT_MAX) * REGULATOR_DAC_GAIN_CODE + REGULATOR_DAC_OFFSET_CODE);
    return LoadOutput_SetDacCode(dac_code);
}
