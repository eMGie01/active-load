#ifndef REGULATOR_H
#define REGULATOR_H

#include "stm32l1xx_hal.h"

HAL_StatusTypeDef Regulator_Run50ms(void);
float Regulator_Compensate(float current, float temperature);

#endif // REGULATOR_H
