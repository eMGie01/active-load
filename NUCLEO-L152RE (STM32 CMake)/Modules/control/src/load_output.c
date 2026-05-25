#include "load_output.h"
#include "app_state.h"
#include "dac8830.h"

#define LOAD_OUTPUT_V_MAX_DAC 3.999f

static DAC8830_Handle_t s_dac;

HAL_StatusTypeDef LoadOutput_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    if ( !dac8830_Init(&s_dac, hspi, cs_port, cs_pin) )
    {
        return HAL_ERROR;
    }

    AppState_SetDacOutputValue(0.0f);
    return HAL_OK;
}

HAL_StatusTypeDef LoadOutput_SetDacCode(uint16_t code)
{
    HAL_StatusTypeDef status = dac8830_SetCode(&s_dac, code);

    if ( status == HAL_OK )
    {
        float dac_voltage = ((float)code / 0xFFFEu) * LOAD_OUTPUT_V_MAX_DAC;
        AppState_SetDacOutputValue(dac_voltage);
        AppState_UpdateCrc();
    }

    return status;
}

HAL_StatusTypeDef LoadOutput_SetZero(void)
{
    return LoadOutput_SetDacCode(0U);
}
