#include "logger.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_uart.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define LOG_RB_SIZE 2048

static UART_HandleTypeDef * s_huart;
static uint8_t s_rb[LOG_RB_SIZE];
static volatile uint16_t s_head;
static volatile uint16_t s_tail;
static volatile uint16_t s_len;
static volatile bool s_busy;
static volatile bool s_init;
static volatile uint32_t s_dropped;
static volatile uint32_t s_err_count;


static size_t
rb_push(const uint8_t * data, size_t count)
{
    if ( !data || count == 0U )
    {
        return 0U;
    }

    size_t index_ = 0;
    while ( index_ < count )
    {
        uint16_t next_ = (uint16_t)((s_head + 1U) % LOG_RB_SIZE);
        if ( next_ == s_tail ) break;
        s_rb[s_head] = data[index_++];
        s_head = next_;
    }
    return index_;
}


static size_t
rb_peek_chunk(uint8_t ** ptr)
{
    uint16_t head_, tail_;

    if ( ptr == NULL ) return 0U;
    *ptr = NULL;

    head_ = s_head;
    tail_ = s_tail;

    if ( head_ == tail_ ) return 0U;

    *ptr = &s_rb[tail_];

    return (head_ > tail_) ? (size_t)(head_ - tail_) : (size_t)(LOG_RB_SIZE - tail_);
}


static void 
rb_kick()
{
    uint8_t * ptr_ = NULL;
    size_t len_ = 0;

    __disable_irq();
    if ( !s_init || s_busy ) { __enable_irq(); return; }

    len_ = rb_peek_chunk(&ptr_);
    if ( len_ == 0U ) { __enable_irq(); return; }
    
    s_busy = true;
    s_len = (uint16_t)len_;

    __enable_irq();

    if ( HAL_OK != HAL_UART_Transmit_DMA(s_huart, ptr_, (uint16_t)len_) )
    {
        __disable_irq();
        s_busy = false;
        __enable_irq();
        s_err_count++;
    }
}


static void 
rb_consume(size_t count)
{
    __disable_irq();
    s_tail = (uint16_t)((s_tail + count) % LOG_RB_SIZE);
    __enable_irq();
}


static char *
lvl_str(enum log_level_t lvl)
{
    if ( INFO == lvl ) return "I";
    else if ( WARN == lvl ) return "W";
    else if ( ERR == lvl ) return "E";
    else return "N";
}


static void
log_complete()
{
    rb_consume(s_len);
    s_len = 0;
    s_busy = false;
    rb_kick();
}


void
HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if ( huart->Instance != USART2 ) return;
    log_complete();
}


LOG_Status_t
log_Init(UART_HandleTypeDef * huart)
{
    if ( huart == NULL )
    {
        return LOG_INVALID_ARG_ERR;
    }

    s_huart = huart;
    memset(s_rb, 0, (size_t)LOG_RB_SIZE);
    s_head = 0U;
    s_tail = 0U;
    s_len = 0U;
    s_busy = false;
    s_dropped = 0;
    s_err_count = 0;
    s_init = true;

    return LOG_OK;
}


LOG_Status_t 
log_Write(enum log_level_t lvl, const char * tag, const char * fmt, ...)
{
    if ( !s_init || !fmt  )
    {
        return LOG_INVALID_ARG_ERR;
    }

    char line_[128];
    int count_ = 0;
    va_list ap_;

    count_ += snprintf(
        line_ + count_, sizeof(line_) - (size_t)count_, "[%lu][%s][%s]",
        (unsigned long)HAL_GetTick(), lvl_str(lvl), tag ? tag : "-"
    );

    va_start(ap_, fmt);
    count_ += vsnprintf(line_ + count_, sizeof(line_) - (size_t)count_, fmt, ap_);
    va_end(ap_);

    if ( count_ < 0 ) return LOG_RUNTIME_ERR;
    if ( (size_t)count_ > sizeof(line_) - 3U ) count_ = (int)(sizeof(line_) - 3U);

    line_[count_++] = '\r';
    line_[count_++] = '\n';
    line_[count_++] = '\0';

    __disable_irq();
    size_t written_ = rb_push((const uint8_t *)line_, (size_t)count_);
    if ( written_ < (size_t)count_ ) s_dropped += (uint32_t)((size_t)count_ - written_);
    __enable_irq();

    rb_kick();
    return (written_ > 0U) ?  LOG_OK : LOG_RUNTIME_ERR;
}
