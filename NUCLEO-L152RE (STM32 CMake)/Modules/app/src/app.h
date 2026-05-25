/**
 * @file app.h
 * @author Marek Gałeczka
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef APP_H
#define APP_H

typedef enum
{
    APP_INIT_ONGOING = 0,
    APP_INIT_RESTART,
    APP_INIT_FATAL,
    APP_INIT_DONE
} APP_InitStatus_t;

APP_InitStatus_t app_Init();
void app_Notify50msFromIsr(void);
void app_Process(void);

#endif // APP_H
