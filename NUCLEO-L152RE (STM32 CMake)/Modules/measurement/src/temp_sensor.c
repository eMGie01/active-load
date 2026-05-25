#include "temp_sensor.h"
#include "app_state.h"
#include "main.h"
#include "tim.h"
#include "stm32l1xx_hal.h"
#include <stdint.h>

#define TEMP_CONVERSION_DELAY_MS 710U
#define ONEWIRE_SKIP_ROM 0xCCU
#define ONEWIRE_CONVERT_T 0x44U
#define ONEWIRE_READ_SCRATCHPAD 0xBEU

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    size_t state_offset;
    float max_valid_temperature;
} TempSensorBus_t;

static const TempSensorBus_t s_temp_1 = {
    .port = temp_1_GPIO_Port,
    .pin = temp_1_Pin,
    .state_offset = APP_OFFSET_TEMP_1,
    .max_valid_temperature = 4000.0f,
};

static const TempSensorBus_t s_temp_3 = {
    .port = temp_3_GPIO_Port,
    .pin = temp_3_Pin,
    .state_offset = APP_OFFSET_TEMP_3,
    .max_valid_temperature = 2000.0f,
};

static uint8_t s_conversion_active;
static uint32_t s_conversion_start_ms;

static void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim6, 0U);
    while (__HAL_TIM_GET_COUNTER(&htim6) < us) {}
}

static HAL_StatusTypeDef one_wire_reset(const TempSensorBus_t *bus)
{
    HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_RESET);
    delay_us(480U);
    HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_SET);
    delay_us(70U);
    GPIO_PinState state = HAL_GPIO_ReadPin(bus->port, bus->pin);
    delay_us(410U);

    return (state == GPIO_PIN_RESET) ? HAL_OK : HAL_ERROR;
}

static void one_wire_write_bit(const TempSensorBus_t *bus, uint8_t value)
{
    if ( value )
    {
        HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_RESET);
        delay_us(6U);
        HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_SET);
        delay_us(64U);
    }
    else
    {
        HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_RESET);
        delay_us(60U);
        HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_SET);
        delay_us(10U);
    }
}

static uint8_t one_wire_read_bit(const TempSensorBus_t *bus)
{
    HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_RESET);
    delay_us(6U);
    HAL_GPIO_WritePin(bus->port, bus->pin, GPIO_PIN_SET);
    delay_us(9U);
    uint8_t value = (HAL_GPIO_ReadPin(bus->port, bus->pin) == GPIO_PIN_SET) ? 1U : 0U;
    delay_us(55U);
    return value;
}

static void one_wire_write_byte(const TempSensorBus_t *bus, uint8_t byte)
{
    for ( uint8_t i = 0U; i < 8U; ++i )
    {
        one_wire_write_bit(bus, (uint8_t)(byte & 0x01U));
        byte >>= 1U;
    }
}

static uint8_t one_wire_read_byte(const TempSensorBus_t *bus)
{
    uint8_t value = 0U;

    for ( uint8_t i = 0U; i < 8U; ++i )
    {
        value >>= 1U;
        if ( one_wire_read_bit(bus) )
        {
            value |= 0x80U;
        }
    }

    return value;
}

static void start_conversion(const TempSensorBus_t *bus)
{
    if ( one_wire_reset(bus) == HAL_OK )
    {
        one_wire_write_byte(bus, ONEWIRE_SKIP_ROM);
        one_wire_write_byte(bus, ONEWIRE_CONVERT_T);
    }
}

static void read_temperature(const TempSensorBus_t *bus)
{
    if ( one_wire_reset(bus) != HAL_OK )
    {
        return;
    }

    one_wire_write_byte(bus, ONEWIRE_SKIP_ROM);
    one_wire_write_byte(bus, ONEWIRE_READ_SCRATCHPAD);

    uint8_t scratchpad[9] = {0U};
    for ( uint8_t i = 0U; i < sizeof(scratchpad); ++i )
    {
        scratchpad[i] = one_wire_read_byte(bus);
    }

    uint16_t combined = ((uint16_t)scratchpad[1] << 8) | scratchpad[0];
    float temperature = (float)combined / 16.0f;

    if ( temperature < bus->max_valid_temperature )
    {
        AppState_SetFloat(bus->state_offset, temperature);
        AppState_UpdateCrc();
    }
}

void TempSensor_Init(void)
{
    s_conversion_active = 0U;
    s_conversion_start_ms = 0U;
}

void TempSensor_Process(void)
{
    if ( s_conversion_active == 0U )
    {
        start_conversion(&s_temp_1);
        start_conversion(&s_temp_3);
        s_conversion_start_ms = HAL_GetTick();
        s_conversion_active = 1U;
        return;
    }

    if ( (HAL_GetTick() - s_conversion_start_ms) >= TEMP_CONVERSION_DELAY_MS )
    {
        read_temperature(&s_temp_1);
        read_temperature(&s_temp_3);
        s_conversion_active = 0U;
    }
}
