#include "param_table.h"
#include "app_state.h"

static const ParamEntry_t s_params[] = {
    { .id = APP_OFFSET_SETPOINT, .offset = APP_OFFSET_SETPOINT, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 180.0f, .default_value = 0.7f },
    { .id = APP_OFFSET_PARAMETER, .offset = APP_OFFSET_PARAMETER, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 180.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_CURRENT, .offset = APP_OFFSET_CURRENT, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 15.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_BATTERY_VOLTAGE, .offset = APP_OFFSET_BATTERY_VOLTAGE, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 20.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_DAC_OUTPUT, .offset = APP_OFFSET_DAC_OUTPUT, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 3.999f, .default_value = 0.0f },
    { .id = APP_OFFSET_TEMP_1, .offset = APP_OFFSET_TEMP_1, .type = PARAM_TYPE_FLOAT, .min_value = -55.0f, .max_value = 200.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_TEMP_2, .offset = APP_OFFSET_TEMP_2, .type = PARAM_TYPE_FLOAT, .min_value = -55.0f, .max_value = 200.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_TEMP_3, .offset = APP_OFFSET_TEMP_3, .type = PARAM_TYPE_FLOAT, .min_value = -55.0f, .max_value = 200.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_TEMP_4, .offset = APP_OFFSET_TEMP_4, .type = PARAM_TYPE_FLOAT, .min_value = -55.0f, .max_value = 200.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_MODE, .offset = APP_OFFSET_MODE, .type = PARAM_TYPE_U8, .min_value = 0.0f, .max_value = 255.0f, .default_value = (float)'C' },
    { .id = APP_OFFSET_ON_OFF, .offset = APP_OFFSET_ON_OFF, .type = PARAM_TYPE_U8, .min_value = 0.0f, .max_value = 1.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_LOGGING, .offset = APP_OFFSET_LOGGING, .type = PARAM_TYPE_U8, .min_value = 0.0f, .max_value = 1.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_LOGGING_BUFFER_SIZE, .offset = APP_OFFSET_LOGGING_BUFFER_SIZE, .type = PARAM_TYPE_U16, .min_value = 0.0f, .max_value = 1024.0f, .default_value = 350.0f },
    { .id = APP_OFFSET_LOGGING_SPEED, .offset = APP_OFFSET_LOGGING_SPEED, .type = PARAM_TYPE_U16, .min_value = 1.0f, .max_value = 60000.0f, .default_value = 100.0f },
    { .id = APP_OFFSET_WIRE_RESISTANCE, .offset = APP_OFFSET_WIRE_RESISTANCE, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 10.0f, .default_value = 0.0f },
    { .id = APP_OFFSET_KP, .offset = APP_OFFSET_KP, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 100.0f, .default_value = 0.1f },
    { .id = APP_OFFSET_KI, .offset = APP_OFFSET_KI, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 100.0f, .default_value = 0.1f },
    { .id = APP_OFFSET_KD, .offset = APP_OFFSET_KD, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 100.0f, .default_value = 0.1f },
    { .id = APP_OFFSET_TIME_STOP, .offset = APP_OFFSET_TIME_STOP, .type = PARAM_TYPE_FLOAT, .min_value = 0.0f, .max_value = 100000.0f, .default_value = 0.0f },
};

const ParamEntry_t *
PARAM_TABLE_Get(void)
{
    return s_params;
}


size_t
PARAM_TABLE_Count(void)
{
    return sizeof(s_params) / sizeof(s_params[0]);
}


const ParamEntry_t *
PARAM_TABLE_Find(uint16_t id)
{
    for ( size_t i = 0U; i < PARAM_TABLE_Count(); ++i )
    {
        if ( s_params[i].id == id )
        {
            return &s_params[i];
        }
    }

    return NULL;
}
