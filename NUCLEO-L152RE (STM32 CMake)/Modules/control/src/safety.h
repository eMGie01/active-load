#ifndef SAFETY_H
#define SAFETY_H

#include <stdint.h>

typedef enum
{
    SAFETY_REASON_NONE = 0,
    SAFETY_REASON_UNDERVOLTAGE,
    SAFETY_REASON_CHARGE_LIMIT,
    SAFETY_REASON_OVERTEMPERATURE,
    SAFETY_REASON_TIME_LIMIT
} SafetyReason_t;

void Safety_Init(void);
void Safety_Process(void);
SafetyReason_t Safety_GetLastReason(void);

#endif // SAFETY_H
