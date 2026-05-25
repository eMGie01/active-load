#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include "stm32l1xx_hal.h"

HAL_StatusTypeDef ConfigStore_LoadPartial(void);
HAL_StatusTypeDef ConfigStore_SavePartial(void);

#endif // CONFIG_STORE_H
