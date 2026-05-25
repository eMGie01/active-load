#include "app_state.h"
#include <math.h>
#include <string.h>

static AppLegacyFrame_t s_frame;

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

void AppState_InitDefaults(void)
{
    memset(&s_frame, 0, sizeof(s_frame));
    s_frame.header = APP_LEGACY_HEADER_VALUE;
    s_frame.length = 0x26U;

    AppState_SetMode((uint8_t)'C');
    AppState_SetOnOff(0U);
    AppState_SetLogging(0U);
    AppState_SetLoggingBufferSize(350U);
    AppState_SetLoggingSpeed(100U);
    AppState_SetFloat(APP_OFFSET_WIRE_RESISTANCE, 0.0f);
    AppState_SetFloat(APP_OFFSET_KP, 0.1f);
    AppState_SetFloat(APP_OFFSET_KI, 0.1f);
    AppState_SetFloat(APP_OFFSET_KD, 0.1f);
    AppState_SetFloat(APP_OFFSET_TIME_STOP, 0.0f);
    AppState_SetSetpoint(0.7f);
    AppState_UpdateCrc();
}

AppLegacyFrame_t *AppState_Frame(void)
{
    return &s_frame;
}

const AppLegacyFrame_t *AppState_FrameConst(void)
{
    return &s_frame;
}

uint32_t AppState_CalculateCrc(void)
{
    uint32_t crc = (uint32_t)s_frame.header + (uint32_t)s_frame.length;

    for ( size_t i = 0U; i < sizeof(s_frame.data); ++i )
    {
        crc += (uint32_t)s_frame.data[i];
    }

    return crc;
}

void AppState_UpdateCrc(void)
{
    s_frame.crc = AppState_CalculateCrc();
}

float AppState_GetFloat(size_t offset)
{
    float value = 0.0f;

    if ( offset + sizeof(value) <= sizeof(s_frame.data) )
    {
        memcpy(&value, &s_frame.data[offset], sizeof(value));
    }

    return value;
}

void AppState_SetFloat(size_t offset, float value)
{
    if ( offset + sizeof(value) <= sizeof(s_frame.data) )
    {
        memcpy(&s_frame.data[offset], &value, sizeof(value));
    }
}

uint8_t AppState_GetU8(size_t offset)
{
    if ( offset < sizeof(s_frame.data) )
    {
        return s_frame.data[offset];
    }

    return 0U;
}

void AppState_SetU8(size_t offset, uint8_t value)
{
    if ( offset < sizeof(s_frame.data) )
    {
        s_frame.data[offset] = value;
    }
}

uint16_t AppState_GetU16(size_t offset)
{
    uint16_t value = 0U;

    if ( offset + sizeof(value) <= sizeof(s_frame.data) )
    {
        memcpy(&value, &s_frame.data[offset], sizeof(value));
    }

    return value;
}

void AppState_SetU16(size_t offset, uint16_t value)
{
    if ( offset + sizeof(value) <= sizeof(s_frame.data) )
    {
        memcpy(&s_frame.data[offset], &value, sizeof(value));
    }
}

float AppState_GetSetpoint(void)
{
    return AppState_GetFloat(APP_OFFSET_SETPOINT);
}

void AppState_SetSetpoint(float setpoint)
{
    uint8_t mode = AppState_GetMode();

    if ( mode == (uint8_t)'C' )
    {
        setpoint = clamp_float(setpoint, 0.0f, 15.0f);
    }
    else if ( mode == (uint8_t)'P' )
    {
        setpoint = clamp_float(setpoint, 0.0f, 180.0f);
    }
    else if ( mode == (uint8_t)'R' )
    {
        setpoint = clamp_float(setpoint, 0.8f, 80.0f);
    }

    setpoint = roundf(setpoint * 1000.0f) / 1000.0f;
    AppState_SetFloat(APP_OFFSET_SETPOINT, setpoint);
}

float AppState_GetCurrent(void)
{
    return AppState_GetFloat(APP_OFFSET_CURRENT);
}

void AppState_SetCurrentValue(float current)
{
    AppState_SetFloat(APP_OFFSET_CURRENT, current);
}

float AppState_GetBatteryVoltage(void)
{
    return AppState_GetFloat(APP_OFFSET_BATTERY_VOLTAGE);
}

void AppState_SetBatteryVoltageValue(float voltage)
{
    AppState_SetFloat(APP_OFFSET_BATTERY_VOLTAGE, voltage);
}

float AppState_GetDacOutput(void)
{
    return AppState_GetFloat(APP_OFFSET_DAC_OUTPUT);
}

void AppState_SetDacOutputValue(float voltage)
{
    AppState_SetFloat(APP_OFFSET_DAC_OUTPUT, voltage);
}

float AppState_GetParameter(void)
{
    return AppState_GetFloat(APP_OFFSET_PARAMETER);
}

void AppState_UpdateParameter(void)
{
    float parameter = 0.0f;
    float voltage = AppState_GetBatteryVoltage();
    float current = AppState_GetCurrent();
    uint8_t mode = AppState_GetMode();

    if ( mode == (uint8_t)'C' )
    {
        parameter = current;
    }
    else if ( mode == (uint8_t)'R' && current != 0.0f )
    {
        parameter = voltage / current;
    }
    else if ( mode == (uint8_t)'P' )
    {
        parameter = voltage * current;
    }

    AppState_SetFloat(APP_OFFSET_PARAMETER, parameter);
}

uint8_t AppState_GetMode(void)
{
    return AppState_GetU8(APP_OFFSET_MODE);
}

void AppState_SetMode(uint8_t mode)
{
    AppState_SetU8(APP_OFFSET_MODE, mode);
}

uint8_t AppState_GetOnOff(void)
{
    return AppState_GetU8(APP_OFFSET_ON_OFF);
}

void AppState_SetOnOff(uint8_t on_off)
{
    AppState_SetU8(APP_OFFSET_ON_OFF, on_off);
}

uint8_t AppState_GetLogging(void)
{
    return AppState_GetU8(APP_OFFSET_LOGGING);
}

void AppState_SetLogging(uint8_t logging)
{
    AppState_SetU8(APP_OFFSET_LOGGING, logging);
}

uint16_t AppState_GetLoggingBufferSize(void)
{
    return AppState_GetU16(APP_OFFSET_LOGGING_BUFFER_SIZE);
}

void AppState_SetLoggingBufferSize(uint16_t size)
{
    AppState_SetU16(APP_OFFSET_LOGGING_BUFFER_SIZE, size);
}

uint16_t AppState_GetLoggingSpeed(void)
{
    return AppState_GetU16(APP_OFFSET_LOGGING_SPEED);
}

void AppState_SetLoggingSpeed(uint16_t speed)
{
    AppState_SetU16(APP_OFFSET_LOGGING_SPEED, speed);
}
