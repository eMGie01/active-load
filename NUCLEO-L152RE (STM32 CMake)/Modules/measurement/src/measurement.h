/**
 * @file measurement.h
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include "mcp3561t.h"


MCP3561T_Status_t MEAS_CurrentInit();
MCP3561T_Status_t MEAS_VoltageInit();
MCP3561T_Status_t MEAS_Poll(void);
MCP3561T_Status_t MEAS_ReadCurrent(float *current);
MCP3561T_Status_t MEAS_ReadVoltage(float *voltage);
float MEAS_GetChargeCoulombs(void);


#endif // MEASUREMENT_H
