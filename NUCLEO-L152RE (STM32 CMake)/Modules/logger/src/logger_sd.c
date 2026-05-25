#include "logger_sd.h"
#include "app_state.h"
#include "fatfs.h"
#include "main.h"
#include "measurement.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

#define LOG_SD_MAX_BUFFER_ROWS 64U
#define LOG_SD_ROW_SIZE 256U
#define LOG_SD_DATE_TIME_LENGTH 16U

static const char *TAG = "SD";
static char s_file_path[48];
static char s_rows[LOG_SD_MAX_BUFFER_ROWS][LOG_SD_ROW_SIZE];
static uint16_t s_row_count;
static uint16_t s_tick_count;
static uint8_t s_prev_logging;
static uint8_t s_file_ready;
static uint32_t s_time_base_ms;

static void sd_attach(void)
{
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static void sd_detach(void)
{
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

static void get_date_time(char *date_time_str, size_t size)
{
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;

    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    snprintf(date_time_str, size, "%02d%02d%02d%02d", date.Month, date.Date, time.Hours, time.Minutes);
}

static LOG_Status_t create_folder_with_csv(void)
{
    char folder_name[LOG_SD_DATE_TIME_LENGTH];
    FRESULT res;
    FIL file;

    sd_attach();
    HAL_Delay(20U);

    res = f_mount(&USERFatFS, USERPath, 1);
    if ( res != FR_OK )
    {
        log_Write(ERR, TAG, "f_mount error (%d)", res);
        AppState_SetLogging(0U);
        sd_detach();
        return LOG_RUNTIME_ERR;
    }

    get_date_time(folder_name, sizeof(folder_name));

    res = f_mkdir(folder_name);
    if ( res != FR_OK && res != FR_EXIST )
    {
        log_Write(ERR, TAG, "f_mkdir %s error (%d)", folder_name, res);
        f_mount(NULL, USERPath, 0);
        sd_detach();
        return LOG_RUNTIME_ERR;
    }

    snprintf(s_file_path, sizeof(s_file_path), "%s/%s.csv", folder_name, folder_name);

    res = f_open(&file, s_file_path, FA_WRITE | FA_CREATE_ALWAYS);
    if ( res != FR_OK )
    {
        log_Write(ERR, TAG, "f_open %s error (%d)", s_file_path, res);
        f_mount(NULL, USERPath, 0);
        sd_detach();
        return LOG_RUNTIME_ERR;
    }

    const char header[] = "time, setpoint, parameter, current, battery voltage, DAC, temp_1, temp_2, ON/OFF, qlmb\r\n";
    UINT bytes_written = 0U;
    res = f_write(&file, header, strlen(header), &bytes_written);
    f_close(&file);
    f_mount(NULL, USERPath, 0);
    sd_detach();

    if ( res != FR_OK )
    {
        log_Write(ERR, TAG, "header write error (%d)", res);
        return LOG_RUNTIME_ERR;
    }

    s_file_ready = 1U;
    return LOG_OK;
}

static uint16_t desired_buffer_rows(void)
{
    uint16_t rows = AppState_GetLoggingBufferSize();

    if ( rows == 0U || rows > LOG_SD_MAX_BUFFER_ROWS )
    {
        rows = LOG_SD_MAX_BUFFER_ROWS;
    }

    return rows;
}

static LOG_Status_t write_buffer_to_csv(void)
{
    if ( s_row_count == 0U || s_file_ready == 0U )
    {
        return LOG_OK;
    }

    FRESULT res;
    FIL file;

    sd_attach();
    res = f_mount(&USERFatFS, USERPath, 1);
    if ( res == FR_OK )
    {
        res = f_open(&file, s_file_path, FA_WRITE | FA_OPEN_APPEND);
    }

    if ( res == FR_OK )
    {
        UINT bytes_written = 0U;
        for ( uint16_t i = 0U; i < s_row_count; ++i )
        {
            res = f_write(&file, s_rows[i], strlen(s_rows[i]), &bytes_written);
            if ( res != FR_OK )
            {
                break;
            }
        }
        f_close(&file);
    }

    f_mount(NULL, USERPath, 0);
    sd_detach();

    if ( res != FR_OK )
    {
        log_Write(ERR, TAG, "csv append error (%d)", res);
        return LOG_RUNTIME_ERR;
    }

    for ( uint16_t i = 0U; i < s_row_count; ++i )
    {
        memset(s_rows[i], 0, sizeof(s_rows[i]));
    }
    s_row_count = 0U;
    return LOG_OK;
}

static void add_data_to_buffer(void)
{
    if ( s_prev_logging == 0U && AppState_GetLogging() == 1U )
    {
        s_prev_logging = 1U;
        s_row_count = 0U;
        s_time_base_ms = HAL_GetTick();
        if ( create_folder_with_csv() != LOG_OK )
        {
            return;
        }
    }

    if ( s_file_ready == 0U )
    {
        return;
    }

    float time_s = (float)(HAL_GetTick() - s_time_base_ms) / 1000.0f;

    int len = snprintf(
        s_rows[s_row_count],
        sizeof(s_rows[s_row_count]),
        "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%.2f,%d,%.2f\r\n",
        time_s,
        AppState_GetSetpoint(),
        AppState_GetParameter(),
        AppState_GetCurrent(),
        AppState_GetBatteryVoltage(),
        AppState_GetDacOutput(),
        AppState_GetFloat(APP_OFFSET_TEMP_1),
        AppState_GetFloat(APP_OFFSET_TEMP_3),
        AppState_GetOnOff(),
        MEAS_GetChargeCoulombs()
    );

    if ( len < 0 || (size_t)len >= sizeof(s_rows[s_row_count]) )
    {
        log_Write(ERR, TAG, "csv row encode error");
        return;
    }

    s_row_count++;

    if ( s_row_count >= desired_buffer_rows() )
    {
        (void)write_buffer_to_csv();
    }
}

LOG_Status_t
log_sd_Init(void)
{
    memset(s_file_path, 0, sizeof(s_file_path));
    memset(s_rows, 0, sizeof(s_rows));
    s_row_count = 0U;
    s_tick_count = 0U;
    s_prev_logging = 0U;
    s_file_ready = 0U;
    s_time_base_ms = 0U;
    return LOG_OK;
}

void
log_sd_Process50ms(void)
{
    if ( AppState_GetLogging() != 1U )
    {
        s_prev_logging = 0U;
        s_tick_count = 0U;
        return;
    }

    s_tick_count++;

    uint16_t speed_ms = 100U;
    if ( s_time_base_ms != 0U )
    {
        float minutes_elapsed = (float)(HAL_GetTick() - s_time_base_ms) / (1000.0f * 60.0f);
        if ( minutes_elapsed >= 1.0f )
        {
            speed_ms = AppState_GetLoggingSpeed();
        }
    }

    uint16_t ticks_per_sample = speed_ms / 50U;
    if ( ticks_per_sample == 0U )
    {
        ticks_per_sample = 1U;
    }

    if ( s_tick_count >= ticks_per_sample )
    {
        s_tick_count = 0U;
        add_data_to_buffer();
    }
}
