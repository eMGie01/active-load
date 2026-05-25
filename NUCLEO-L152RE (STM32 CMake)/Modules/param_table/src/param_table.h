#ifndef PARAM_TABLE_H
#define PARAM_TABLE_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    PARAM_TYPE_U8 = 0,
    PARAM_TYPE_U16,
    PARAM_TYPE_FLOAT
} ParamType_t;

typedef struct
{
    uint16_t id;
    size_t offset;
    ParamType_t type;
    float min_value;
    float max_value;
    float default_value;
} ParamEntry_t;

const ParamEntry_t *PARAM_TABLE_Get(void);
size_t PARAM_TABLE_Count(void);
const ParamEntry_t *PARAM_TABLE_Find(uint16_t id);

#endif // PARAM_TABLE_H
