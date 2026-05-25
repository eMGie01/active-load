#ifndef APP_STATE_H
#define APP_STATE_H

#include <stddef.h>
#include <stdint.h>

#define APP_LEGACY_HEADER_VALUE 0x77U
#define APP_LEGACY_DATA_SIZE 67U

#define APP_OFFSET_SETPOINT 0U
#define APP_OFFSET_PARAMETER 4U
#define APP_OFFSET_CURRENT 8U
#define APP_OFFSET_BATTERY_VOLTAGE 12U
#define APP_OFFSET_DAC_OUTPUT 16U
#define APP_OFFSET_TEMP_1 20U
#define APP_OFFSET_TEMP_2 24U
#define APP_OFFSET_TEMP_3 28U
#define APP_OFFSET_TEMP_4 32U
#define APP_OFFSET_MODE 36U
#define APP_OFFSET_ON_OFF 37U
#define APP_OFFSET_LOGGING 38U
#define APP_OFFSET_LOGGING_BUFFER_SIZE 39U
#define APP_OFFSET_LOGGING_SPEED 41U
#define APP_OFFSET_WIRE_RESISTANCE 43U
#define APP_OFFSET_KP 47U
#define APP_OFFSET_KI 51U
#define APP_OFFSET_KD 55U
#define APP_OFFSET_TIME_STOP 59U

typedef struct
{
    uint8_t header;
    uint8_t length;
    uint8_t data[APP_LEGACY_DATA_SIZE];
    uint32_t crc;
} AppLegacyFrame_t;

void AppState_InitDefaults(void);
AppLegacyFrame_t *AppState_Frame(void);
const AppLegacyFrame_t *AppState_FrameConst(void);
uint32_t AppState_CalculateCrc(void);
void AppState_UpdateCrc(void);

float AppState_GetFloat(size_t offset);
void AppState_SetFloat(size_t offset, float value);
uint8_t AppState_GetU8(size_t offset);
void AppState_SetU8(size_t offset, uint8_t value);
uint16_t AppState_GetU16(size_t offset);
void AppState_SetU16(size_t offset, uint16_t value);

float AppState_GetSetpoint(void);
void AppState_SetSetpoint(float setpoint);
float AppState_GetCurrent(void);
void AppState_SetCurrentValue(float current);
float AppState_GetBatteryVoltage(void);
void AppState_SetBatteryVoltageValue(float voltage);
float AppState_GetDacOutput(void);
void AppState_SetDacOutputValue(float voltage);
float AppState_GetParameter(void);
void AppState_UpdateParameter(void);
uint8_t AppState_GetMode(void);
void AppState_SetMode(uint8_t mode);
uint8_t AppState_GetOnOff(void);
void AppState_SetOnOff(uint8_t on_off);
uint8_t AppState_GetLogging(void);
void AppState_SetLogging(uint8_t logging);
uint16_t AppState_GetLoggingBufferSize(void);
void AppState_SetLoggingBufferSize(uint16_t size);
uint16_t AppState_GetLoggingSpeed(void);
void AppState_SetLoggingSpeed(uint16_t speed);

#endif // APP_STATE_H
