#ifndef COMMS_H
#define COMMS_H

#include "stm32l1xx_hal.h"

HAL_StatusTypeDef COMMS_StartRx(UART_HandleTypeDef *huart);
void COMMS_Process(void);
uint32_t COMMS_GetAcceptedFrames(void);
uint32_t COMMS_GetRejectedFrames(void);

#endif // COMMS_H
