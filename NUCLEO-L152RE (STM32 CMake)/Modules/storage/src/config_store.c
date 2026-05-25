#include "config_store.h"
#include "app_state.h"
#include "stm32l1xx_hal_flash_ex.h"
#include <stdint.h>
#include <string.h>

#define EEPROM_ADDR_HEADER     (FLASH_EEPROM_BASE)
#define EEPROM_ADDR_SETPOINT   (EEPROM_ADDR_HEADER + 4U)
#define EEPROM_ADDR_MODE       (EEPROM_ADDR_SETPOINT + 4U)
#define EEPROM_ADDR_ONOFF      (EEPROM_ADDR_MODE + 4U)
#define EEPROM_ADDR_LOGGING    (EEPROM_ADDR_ONOFF + 4U)
#define EEPROM_ADDR_BUF_SIZE   (EEPROM_ADDR_LOGGING + 4U)
#define EEPROM_ADDR_SPEED      (EEPROM_ADDR_BUF_SIZE + 4U)
#define EEPROM_ADDR_DATA43     (EEPROM_ADDR_SPEED + 4U)
#define EEPROM_ADDR_DATA47     (EEPROM_ADDR_DATA43 + 4U)
#define EEPROM_ADDR_DATA51     (EEPROM_ADDR_DATA47 + 4U)
#define EEPROM_ADDR_DATA55     (EEPROM_ADDR_DATA51 + 4U)
#define EEPROM_ADDR_DATA59     (EEPROM_ADDR_DATA55 + 4U)

static uint32_t float_to_u32(float value)
{
    uint32_t raw = 0U;
    memcpy(&raw, &value, sizeof(raw));
    return raw;
}

static float u32_to_float(uint32_t raw)
{
    float value = 0.0f;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

static HAL_StatusTypeDef write_word(uint32_t address, uint32_t value)
{
    return HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, address, value);
}

HAL_StatusTypeDef ConfigStore_SavePartial(void)
{
    HAL_StatusTypeDef status = HAL_FLASHEx_DATAEEPROM_Unlock();

    if ( status != HAL_OK )
    {
        return status;
    }

    const AppLegacyFrame_t *frame = AppState_FrameConst();

    status = write_word(EEPROM_ADDR_HEADER, frame->header);
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_SETPOINT, float_to_u32(AppState_GetSetpoint()));
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_MODE, AppState_GetMode());
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_ONOFF, AppState_GetOnOff());
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_LOGGING, AppState_GetLogging());
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_BUF_SIZE, AppState_GetLoggingBufferSize());
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_SPEED, AppState_GetLoggingSpeed());
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_DATA43, float_to_u32(AppState_GetFloat(APP_OFFSET_WIRE_RESISTANCE)));
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_DATA47, float_to_u32(AppState_GetFloat(APP_OFFSET_KP)));
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_DATA51, float_to_u32(AppState_GetFloat(APP_OFFSET_KI)));
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_DATA55, float_to_u32(AppState_GetFloat(APP_OFFSET_KD)));
    if ( status == HAL_OK ) status = write_word(EEPROM_ADDR_DATA59, float_to_u32(AppState_GetFloat(APP_OFFSET_TIME_STOP)));

    HAL_StatusTypeDef lock_status = HAL_FLASHEx_DATAEEPROM_Lock();
    return (status == HAL_OK) ? lock_status : status;
}

HAL_StatusTypeDef ConfigStore_LoadPartial(void)
{
    uint8_t header = *(uint8_t *)EEPROM_ADDR_HEADER;

    if ( header != APP_LEGACY_HEADER_VALUE )
    {
        AppState_UpdateCrc();
        return HAL_ERROR;
    }

    AppLegacyFrame_t *frame = AppState_Frame();
    frame->header = header;
    frame->length = 0x26U;

    AppState_SetSetpoint(u32_to_float(*(uint32_t *)EEPROM_ADDR_SETPOINT));
    AppState_SetMode(*(uint8_t *)EEPROM_ADDR_MODE);
    AppState_SetOnOff(*(uint8_t *)EEPROM_ADDR_ONOFF);
    AppState_SetLogging(*(uint8_t *)EEPROM_ADDR_LOGGING);
    AppState_SetLoggingBufferSize(*(uint16_t *)EEPROM_ADDR_BUF_SIZE);
    AppState_SetLoggingSpeed(*(uint16_t *)EEPROM_ADDR_SPEED);
    AppState_SetFloat(APP_OFFSET_WIRE_RESISTANCE, u32_to_float(*(uint32_t *)EEPROM_ADDR_DATA43));
    AppState_SetFloat(APP_OFFSET_KP, u32_to_float(*(uint32_t *)EEPROM_ADDR_DATA47));
    AppState_SetFloat(APP_OFFSET_KI, u32_to_float(*(uint32_t *)EEPROM_ADDR_DATA51));
    AppState_SetFloat(APP_OFFSET_KD, u32_to_float(*(uint32_t *)EEPROM_ADDR_DATA55));
    AppState_SetFloat(APP_OFFSET_TIME_STOP, u32_to_float(*(uint32_t *)EEPROM_ADDR_DATA59));

    AppState_UpdateCrc();
    return HAL_OK;
}
