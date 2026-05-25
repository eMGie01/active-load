/**
 * @file logger.h
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef LOGGER_H
#define LOGGER_H

#include "usart.h"

enum log_level_t
{
    INFO,
    WARN,
    ERR
};

typedef enum
{
    LOG_OK = 0,
    LOG_INVALID_ARG_ERR,
    LOG_RUNTIME_ERR
} LOG_Status_t;

LOG_Status_t log_Init(UART_HandleTypeDef * huart);
LOG_Status_t log_Write(enum log_level_t lvl, const char * tag, const char * fmt, ...);
// LOG_Status_t log_Error();


#endif // LOGGER_H