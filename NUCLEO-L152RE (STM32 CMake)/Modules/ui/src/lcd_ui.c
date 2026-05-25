#include "lcd_ui.h"
#include "app_state.h"
#include "main.h"
#include "stm32l1xx_hal.h"
#include <stdio.h>

#define UI_ON 1U
#define UI_OFF 0U

#define UI_MODE_CC 0U
#define UI_MODE_CR 1U
#define UI_MODE_CP 2U

typedef enum
{
    LCD_VIEW_DEFAULT = 0,
    LCD_VIEW_TURN,
    LCD_VIEW_MODE,
    LCD_VIEW_LOGGING
} LcdView_t;

static volatile uint8_t s_lcd_update_flag;
static volatile uint8_t s_lcd_display_flag;
static LcdView_t s_display;
static uint8_t s_menu_idx;

static void show_default_view(uint32_t *last_change);
static void show_turn_view(void);
static void show_mode_view(void);
static void show_logging_view(void);

static void lcd_enable_pulse(void)
{
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(5U);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(5U);
}

static void lcd_send_4bits(uint8_t data)
{
    HAL_GPIO_WritePin(DB4_GPIO_Port, DB4_Pin, (data & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DB5_GPIO_Port, DB5_Pin, (data & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DB6_GPIO_Port, DB6_Pin, (data & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DB7_GPIO_Port, DB7_Pin, (data & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    lcd_enable_pulse();
}

static void lcd_send_cmd(uint8_t cmd)
{
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RW_GPIO_Port, RW_Pin, GPIO_PIN_RESET);
    lcd_send_4bits((uint8_t)(cmd >> 4));
    lcd_send_4bits((uint8_t)(cmd & 0x0FU));

    if ( cmd == 0x01U || cmd == 0x02U )
    {
        HAL_Delay(2U);
    }
}

static void lcd_send_data(uint8_t data)
{
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RW_GPIO_Port, RW_Pin, GPIO_PIN_RESET);
    lcd_send_4bits((uint8_t)(data >> 4));
    lcd_send_4bits((uint8_t)(data & 0x0FU));
}

static void redraw_current_view(uint32_t *last_change)
{
    switch ( s_display )
    {
        case LCD_VIEW_DEFAULT:
            show_default_view(last_change);
            break;
        case LCD_VIEW_TURN:
            show_turn_view();
            break;
        case LCD_VIEW_MODE:
            show_mode_view();
            break;
        case LCD_VIEW_LOGGING:
            show_logging_view();
            break;
        default:
            break;
    }
}

static void lcd_send_string(const char *str)
{
    while ( *str )
    {
        lcd_send_data((uint8_t)*str++);
    }
}

static void lcd_display_measurement(void)
{
    char text[10];

    snprintf(text, sizeof(text), "%06.3f", AppState_GetBatteryVoltage());
    lcd_send_cmd(0x80U);
    lcd_send_string(text);
    lcd_send_data((uint8_t)'V');

    snprintf(text, sizeof(text), "%06.3f", AppState_GetCurrent());
    lcd_send_cmd(0x89U);
    lcd_send_string(text);
    lcd_send_data((uint8_t)'A');
}

static void lcd_display_setpoint(void)
{
    char text[10];
    uint8_t mode = AppState_GetMode();

    snprintf(text, sizeof(text), "%06.3f", AppState_GetSetpoint());
    lcd_send_cmd(0xC0U);
    lcd_send_string(text);

    if ( mode == (uint8_t)'C' )
    {
        lcd_send_data((uint8_t)'A');
        lcd_send_data((uint8_t)' ');
    }
    else if ( mode == (uint8_t)'R' )
    {
        lcd_send_data((uint8_t)'O');
        lcd_send_data((uint8_t)' ');
    }
    else
    {
        lcd_send_data((uint8_t)'W');
        lcd_send_data((uint8_t)' ');
    }
}

static void lcd_display_mode(void)
{
    lcd_send_cmd(0xC9U);
    lcd_send_data((uint8_t)'C');
    lcd_send_data(AppState_GetMode());

    lcd_send_cmd(0xCDU);
    if ( AppState_GetOnOff() == 0U )
    {
        lcd_send_string("OFF");
    }
    else
    {
        lcd_send_string(" ON");
    }
}

static void show_default_view(uint32_t *last_change)
{
    lcd_send_cmd(0x01U);
    lcd_display_measurement();
    lcd_display_setpoint();
    lcd_display_mode();
    *last_change = HAL_GetTick();
}

static void show_turn_view(void)
{
    lcd_send_cmd(0x01U);
    lcd_send_cmd(0x86U);
    lcd_send_string("[OFF]");
    lcd_send_cmd(0xC7U);
    lcd_send_string("ON");
    lcd_send_cmd(0xCEU);
    lcd_send_string("O/");
    if ( AppState_GetOnOff() == 0U )
    {
        lcd_send_cmd(0x85U);
        lcd_send_data((uint8_t)'_');
        lcd_send_cmd(0xC5U);
        lcd_send_data((uint8_t)' ');
    }
    else
    {
        lcd_send_cmd(0xC5U);
        lcd_send_data((uint8_t)'_');
        lcd_send_cmd(0x85U);
        lcd_send_data((uint8_t)' ');
    }
}

static void show_mode_view(void)
{
    lcd_send_cmd(0x01U);
    lcd_send_cmd(0x81U);
    lcd_send_string("[CC]");
    lcd_send_cmd(0x8BU);
    lcd_send_string("CR");
    lcd_send_cmd(0xC7U);
    lcd_send_string("CP");
    lcd_send_cmd(0xCFU);
    lcd_send_string("M");

    if ( AppState_GetMode() == (uint8_t)'C' )
    {
        lcd_send_cmd(0x80U); lcd_send_data((uint8_t)'_');
        lcd_send_cmd(0x89U); lcd_send_data((uint8_t)' ');
        lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)' ');
    }
    else if ( AppState_GetMode() == (uint8_t)'R' )
    {
        lcd_send_cmd(0x80U); lcd_send_data((uint8_t)' ');
        lcd_send_cmd(0x89U); lcd_send_data((uint8_t)'_');
        lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)' ');
    }
    else
    {
        lcd_send_cmd(0x80U); lcd_send_data((uint8_t)' ');
        lcd_send_cmd(0x89U); lcd_send_data((uint8_t)' ');
        lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)'_');
    }
}

static void show_logging_view(void)
{
    lcd_send_cmd(0x01U);
    lcd_send_cmd(0x86U);
    lcd_send_string("[OFF]");
    lcd_send_cmd(0xC7U);
    lcd_send_string("ON");
    lcd_send_cmd(0xCFU);
    lcd_send_string("L");
    if ( AppState_GetLogging() == 0U )
    {
        lcd_send_cmd(0x85U); lcd_send_data((uint8_t)'_');
        lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)' ');
    }
    else
    {
        lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)'_');
        lcd_send_cmd(0x85U); lcd_send_data((uint8_t)' ');
    }
}

void LcdUi_Init(void)
{
    s_lcd_update_flag = 0U;
    s_lcd_display_flag = 0U;
    s_display = LCD_VIEW_DEFAULT;
    s_menu_idx = 0U;

    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RW_GPIO_Port, RW_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DB4_GPIO_Port, DB4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DB5_GPIO_Port, DB5_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DB6_GPIO_Port, DB6_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DB7_GPIO_Port, DB7_Pin, GPIO_PIN_RESET);

    HAL_Delay(50U);
    lcd_send_4bits(0x03U);
    HAL_Delay(10U);
    lcd_send_4bits(0x03U);
    HAL_Delay(5U);
    lcd_send_4bits(0x03U);
    HAL_Delay(1U);
    lcd_send_4bits(0x02U);

    lcd_send_cmd(0x28U);
    lcd_send_cmd(0x0CU);
    lcd_send_cmd(0x06U);
    lcd_send_cmd(0x01U);
    lcd_send_cmd(0x0CU);

    lcd_display_measurement();
    lcd_display_setpoint();
    lcd_display_mode();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint16_t multiplier = 1U;
    static uint32_t last_clk_change = 0U;
    static uint32_t last_sw_change = 0U;
    uint32_t current_time = HAL_GetTick();

    if ( GPIO_Pin == B1_Pin && (current_time - last_sw_change > 500U) )
    {
        switch ( s_display )
        {
            case LCD_VIEW_DEFAULT: s_display = LCD_VIEW_TURN; break;
            case LCD_VIEW_TURN: s_display = LCD_VIEW_MODE; break;
            case LCD_VIEW_MODE: s_display = LCD_VIEW_LOGGING; break;
            case LCD_VIEW_LOGGING:
            default: s_display = LCD_VIEW_DEFAULT; break;
        }
        s_lcd_display_flag = UI_ON;
        last_sw_change = current_time;
    }

    if ( GPIO_Pin == iCLK_exti_Pin && (current_time - last_clk_change > 200U) )
    {
        switch ( s_display )
        {
            case LCD_VIEW_DEFAULT:
                if ( HAL_GPIO_ReadPin(iDT_GPIO_Port, iDT_Pin) != GPIO_PIN_RESET )
                {
                    AppState_SetSetpoint(AppState_GetSetpoint() + (0.001f * (float)multiplier));
                }
                else
                {
                    AppState_SetSetpoint(AppState_GetSetpoint() - (0.001f * (float)multiplier));
                }
                AppState_UpdateCrc();
                break;
            case LCD_VIEW_TURN:
                s_menu_idx = (s_menu_idx >= UI_ON) ? UI_OFF : UI_ON;
                break;
            case LCD_VIEW_MODE:
                s_menu_idx++;
                if ( s_menu_idx > UI_MODE_CP )
                {
                    s_menu_idx = UI_OFF;
                }
                break;
            case LCD_VIEW_LOGGING:
                s_menu_idx = (s_menu_idx >= UI_ON) ? UI_OFF : UI_ON;
                break;
            default:
                break;
        }
        s_lcd_update_flag = UI_ON;
        last_clk_change = current_time;
    }

    if ( GPIO_Pin == iSW_exti_Pin && (current_time - last_sw_change > 250U) )
    {
        switch ( s_display )
        {
            case LCD_VIEW_DEFAULT:
                multiplier *= 10U;
                if ( multiplier >= 10000U )
                {
                    multiplier = 1U;
                }
                break;
            case LCD_VIEW_TURN:
                AppState_SetOnOff((s_menu_idx != 0U) ? 1U : 0U);
                break;
            case LCD_VIEW_MODE:
                if ( s_menu_idx == UI_MODE_CC )
                {
                    AppState_SetMode((uint8_t)'C');
                }
                else if ( s_menu_idx == UI_MODE_CR )
                {
                    AppState_SetMode((uint8_t)'R');
                }
                else
                {
                    AppState_SetMode((uint8_t)'P');
                }
                break;
            case LCD_VIEW_LOGGING:
                AppState_SetLogging((s_menu_idx != 0U) ? 1U : 0U);
                break;
            default:
                break;
        }
        AppState_UpdateCrc();
        s_lcd_update_flag = UI_ON;
        last_sw_change = current_time;
    }
}

void LcdUi_Process(void)
{
    static float prev_setpoint = 0.0f;
    static float prev_current = 0.0f;
    static float prev_battery_voltage = 0.0f;
    static uint8_t prev_mode = (uint8_t)'C';
    static uint8_t prev_onoff = 0U;
    static uint8_t prev_logging = 0U;
    static uint32_t last_change = 0U;
    uint32_t current_time = HAL_GetTick();

    if ( s_lcd_display_flag == UI_ON )
    {
        s_lcd_display_flag = UI_OFF;
        redraw_current_view(&last_change);
    }

    if ( s_lcd_update_flag == UI_ON )
    {
        s_lcd_update_flag = UI_OFF;
        redraw_current_view(&last_change);
    }

    if ( s_display == LCD_VIEW_DEFAULT && (current_time - last_change) >= 3000U )
    {
        if ( prev_setpoint != AppState_GetSetpoint() )
        {
            prev_setpoint = AppState_GetSetpoint();
            lcd_display_setpoint();
        }

        if ( prev_mode != AppState_GetMode() || prev_onoff != AppState_GetOnOff() )
        {
            prev_mode = AppState_GetMode();
            prev_onoff = AppState_GetOnOff();
            lcd_display_mode();
        }

        if ( prev_current != AppState_GetCurrent() || prev_battery_voltage != AppState_GetBatteryVoltage() )
        {
            prev_current = AppState_GetCurrent();
            prev_battery_voltage = AppState_GetBatteryVoltage();
            lcd_display_measurement();
        }

        last_change = current_time;
    }

    if ( s_display == LCD_VIEW_TURN || s_display == LCD_VIEW_LOGGING )
    {
        if ( (prev_onoff != AppState_GetOnOff() && AppState_GetOnOff() == UI_OFF) ||
             (prev_logging != AppState_GetLogging() && AppState_GetLogging() == UI_OFF) )
        {
            lcd_send_cmd(0x85U); lcd_send_data((uint8_t)'_');
            lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)' ');
            prev_onoff = AppState_GetOnOff();
            prev_logging = AppState_GetLogging();
        }
        else if ( (prev_onoff != AppState_GetOnOff() && AppState_GetOnOff() == UI_ON) ||
                  (prev_logging != AppState_GetLogging() && AppState_GetLogging() == UI_ON) )
        {
            lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)'_');
            lcd_send_cmd(0x85U); lcd_send_data((uint8_t)' ');
            prev_onoff = AppState_GetOnOff();
            prev_logging = AppState_GetLogging();
        }
    }

    if ( s_display == LCD_VIEW_MODE )
    {
        if ( prev_mode != AppState_GetMode() && AppState_GetMode() == (uint8_t)'C' )
        {
            lcd_send_cmd(0x80U); lcd_send_data((uint8_t)'_');
            lcd_send_cmd(0x89U); lcd_send_data((uint8_t)' ');
            lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)' ');
            prev_mode = AppState_GetMode();
        }
        else if ( prev_mode != AppState_GetMode() && AppState_GetMode() == (uint8_t)'R' )
        {
            lcd_send_cmd(0x80U); lcd_send_data((uint8_t)' ');
            lcd_send_cmd(0x89U); lcd_send_data((uint8_t)'_');
            lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)' ');
            prev_mode = AppState_GetMode();
        }
        else if ( prev_mode != AppState_GetMode() && AppState_GetMode() == (uint8_t)'P' )
        {
            lcd_send_cmd(0x80U); lcd_send_data((uint8_t)' ');
            lcd_send_cmd(0x89U); lcd_send_data((uint8_t)' ');
            lcd_send_cmd(0xC5U); lcd_send_data((uint8_t)'_');
            prev_mode = AppState_GetMode();
        }
    }
}
