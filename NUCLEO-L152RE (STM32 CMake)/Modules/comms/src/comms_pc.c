#include "comms.h"
#include "app_state.h"
#include "config_store.h"
#include "logger.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define COMMS_RX_BUFFER_SIZE 256U
#define COMMS_PC_FRAME_SIZE 37U
#define COMMS_PC_CRC_OFFSET 33U

static UART_HandleTypeDef *s_huart;
static uint8_t s_rx_byte;
static volatile uint16_t s_head;
static uint16_t s_tail;
static uint8_t s_rx_buffer[COMMS_RX_BUFFER_SIZE];
static uint8_t s_frame[COMMS_PC_FRAME_SIZE];
static uint8_t s_frame_pos;
static bool s_frame_active;
static uint32_t s_accepted_frames;
static uint32_t s_rejected_frames;

static uint16_t rb_next(uint16_t index)
{
    return (uint16_t)((index + 1U) % COMMS_RX_BUFFER_SIZE);
}

static bool rb_pop(uint8_t *byte)
{
    if ( byte == NULL || s_tail == s_head )
    {
        return false;
    }

    *byte = s_rx_buffer[s_tail];
    s_tail = rb_next(s_tail);
    return true;
}

static uint32_t frame_crc(const uint8_t *frame)
{
    uint32_t crc = 0U;

    for ( size_t i = 0U; i < COMMS_PC_CRC_OFFSET; ++i )
    {
        crc += (uint32_t)frame[i];
    }

    return crc;
}

static void apply_frame(const uint8_t *frame)
{
    float value = 0.0f;
    uint16_t value_u16 = 0U;

    AppLegacyFrame_t *state = AppState_Frame();
    state->header = frame[0];
    state->length = frame[1];

    memcpy(&value, &frame[2], sizeof(value));
    AppState_SetSetpoint(value);
    AppState_SetMode(frame[6]);
    AppState_SetOnOff(frame[7]);
    AppState_SetLogging(frame[8]);

    memcpy(&value_u16, &frame[9], sizeof(value_u16));
    AppState_SetLoggingBufferSize(value_u16);

    memcpy(&value_u16, &frame[11], sizeof(value_u16));
    AppState_SetLoggingSpeed(value_u16);

    memcpy(&value, &frame[13], sizeof(value));
    AppState_SetFloat(APP_OFFSET_WIRE_RESISTANCE, value);
    memcpy(&value, &frame[17], sizeof(value));
    AppState_SetFloat(APP_OFFSET_KP, value);
    memcpy(&value, &frame[21], sizeof(value));
    AppState_SetFloat(APP_OFFSET_KI, value);
    memcpy(&value, &frame[25], sizeof(value));
    AppState_SetFloat(APP_OFFSET_KD, value);
    memcpy(&value, &frame[29], sizeof(value));
    AppState_SetFloat(APP_OFFSET_TIME_STOP, value);

    AppState_UpdateCrc();
}

static void process_byte(uint8_t byte)
{
    if ( !s_frame_active )
    {
        if ( byte != APP_LEGACY_HEADER_VALUE )
        {
            return;
        }

        s_frame_active = true;
        s_frame_pos = 0U;
    }

    s_frame[s_frame_pos++] = byte;

    if ( s_frame_pos < COMMS_PC_FRAME_SIZE )
    {
        return;
    }

    uint32_t expected_crc = 0U;
    memcpy(&expected_crc, &s_frame[COMMS_PC_CRC_OFFSET], sizeof(expected_crc));

    if ( frame_crc(s_frame) == expected_crc )
    {
        apply_frame(s_frame);
        (void)ConfigStore_SavePartial();
        s_accepted_frames++;
    }
    else
    {
        s_rejected_frames++;
    }

    s_frame_active = false;
    s_frame_pos = 0U;
}


HAL_StatusTypeDef
COMMS_StartRx(UART_HandleTypeDef *huart)
{
    if ( huart == NULL )
    {
        return HAL_ERROR;
    }

    s_huart = huart;
    s_head = 0U;
    s_tail = 0U;
    s_frame_pos = 0U;
    s_frame_active = false;
    s_accepted_frames = 0U;
    s_rejected_frames = 0U;

    return HAL_UART_Receive_IT(s_huart, &s_rx_byte, 1U);
}


void
COMMS_Process(void)
{
    uint8_t byte = 0U;

    while ( rb_pop(&byte) )
    {
        process_byte(byte);
    }
}


void
HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ( huart != s_huart )
    {
        return;
    }

    uint16_t next = rb_next(s_head);

    if ( next != s_tail )
    {
        s_rx_buffer[s_head] = s_rx_byte;
        s_head = next;
    }

    (void)HAL_UART_Receive_IT(s_huart, &s_rx_byte, 1U);
}


uint32_t
COMMS_GetAcceptedFrames(void)
{
    return s_accepted_frames;
}


uint32_t
COMMS_GetRejectedFrames(void)
{
    return s_rejected_frames;
}
