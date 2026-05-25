# STM32L152RE Load Controller - Refactor Plan

## 📋 Project Overview
**Device Type:** Bare-metal embedded load testing system for battery/power supply characterization

**Features:**
- Voltage monitoring (10.5-15V)
- Current control (0-15A) with PI/PID regulation
- SD card data logging (CSV format)
- User interface: LCD + rotary encoder
- PC communication: UART binary protocol

---

## 🏗️ Architecture Decisions

### Data Management
```c
// Central structure contains ALL state
DataStruct myData;

// All access through get/set functions (no direct access)
setSetpoint(3.5);              // Accessor
float current = getCurrent();  // Getter
```

- **EEPROM persistence:** Automatic save on each PC message
- **Access pattern:** Encapsulated via getter/setter functions

### Timing Model
- **Main loop tick:** 50ms periodic interrupt (TIM2)
- **Regulator execution:** Synchronous with 50ms callback from ISR
- **LCD update:** Every 2 seconds with measured values
- **Error display:** Can show critical errors on LCD
- **Temperature sensing:** Independent periodic measurement (DS18B20)

### State Machine
```
IDLE → MEASURING → REGULATING

States:
- IDLE: Waiting for configuration or OnOff command
- MEASURING: Reading sensors (temperature, voltage, current)
- REGULATING: PID loop active, adjusting DAC output

Safety Actions (NO SHUTDOWN STATE):
- Temperature > threshold → stop regulating (but keep logging)
- Voltage < 11.2V → stop regulating (but keep logging)
- Ampere-hours > limit → stop regulating (but keep logging)
- Communication error → alerting via ERR_OUTPUT module
```

### Logging Strategy
- **Error logging:** Only UART output (ERR_OUTPUT module) - no debug spam
- **Data logging:** Asynchronous buffering to SD card (csv format)
- **Measurement rate:**
  - First 1 minute: 100Hz (20ms intervals)
  - After 1 minute: User-defined rate from config

---

## 📦 Module Architecture

### **FAZA 1 - FUNDAMENTY (Prerequisites)**
These modules are dependencies for everything else.

#### 1️⃣ `my_struct.c/h` - Data Container
```c
// Contains:
// - setpoint, current, voltage, temperature values
// - DAC output, control mode (C/P/R)
// - Config block (Kp, Ki, Kd, timeouts, wire resistance)
// - Getter/setter functions for all fields
// - Thread-safe/ISR-safe access patterns
```
**Depends on:** Nothing
**Used by:** Everything

#### 2️⃣ `uart_utils.c/h` - Low-Level UART
```c
// Functions:
uart_init(USART2, BAUDRATE);           // Setup
uart_send_byte(0xFF);                  // Single byte
uart_send_string("Hello");             // String (no newline)
void uart_rx_callback(uint8_t byte);   // Called by ISR for each byte

// Manages circular RX buffer internally
// Pure I/O layer - NO protocol parsing
```
**Depends on:** HAL GPIO/UART
**Used by:** pc_comms, err_output

#### 3️⃣ `drivers/adc_driver.c/h` - MCP3561 Abstraction
```c
// Measures voltage and current via SPI3
adc_init();
float voltage = adc_read_voltage();  // Returns 10.5-15V
float current = adc_read_current();  // Returns 0-15A

// Internally:
// - SPI3 communication
// - Calibration factors
// - Error handling (timeout, CRC)
```
**Depends on:** HAL SPI3
**Used by:** regulator, main_app

#### 4️⃣ `drivers/dac_driver.c/h` - DAC/MOSFET Control
```c
// Controls MOSFET gate via DAC
dac_init();
dac_set_output(3.5);  // Set to 3.5A

// Internally:
// - Mapping: 0-15A → DAC value range
// - Temperature compensation factor
// - Safety limits (0 to max output)
```
**Depends on:** HAL SPI/I2C/GPIO (depends on hardware)
**Used by:** regulator

---

### **FAZA 2 - Niezależne moduły**
These don't depend on each other, only on Faza 1 and shared data.

#### 5️⃣ `err_output.c/h` - Error Logger
```c
// Log errors/warnings to UART only (no spam)
log_error(ERR_CODE_SD_WRITE_FAIL, "SD write timeout");
log_warning(WARN_HIGH_TEMP, "Temperature 165°C");

// Levels: ERROR (always logged), WARNING (configurable)
// Format: [HH:MM:SS] ERROR: SD write timeout
// Rate-limited to prevent UART overflow
```
**Depends on:** uart_utils, rtc
**Used by:** All modules for error reporting

#### 6️⃣ `lcd_interface.c/h` - Display & Input
```c
// HD44780 LCD + rotary encoder + buttons
lcd_init();
lcd_display_values(voltage, current, mode);  // Update display
lcd_show_error("SD ERROR");                  // Show critical error

// Handles:
// - Page cycling (B1 button)
// - Parameter adjustment (rotary encoder)
// - Mode selection (encoder long press)
// Updated every 2 seconds from main loop
```
**Depends on:** HAL GPIO
**Used by:** main_app for user feedback

#### 7️⃣ `drivers/temp_sensor.c/h` - DS18B20
```c
// 1-wire protocol abstraction
temp_sensor_init();
float temp[4] = temp_sensor_read_all();  // Read 4 sensors
bool timeout = temp_sensor_check_timeout();

// Handles:
// - 1-wire bus communication
// - Conversion timing (~750ms)
// - Error detection
// - Timeout if sensor fails
```
**Depends on:** HAL GPIO (1-wire on GPIO)
**Used by:** regulator, system_state

---

### **FAZA 3 - Komunikacja**
These handle external data exchange.

#### 8️⃣ `pc_comms.c/h` - UART Protocol
```c
// Binary protocol: [0x77] [Length] [Data] [CRC]
// Called from main loop periodically
pc_comms_process();  // Check for incoming messages

// On valid message:
// - Parse command (setSetpoint, setMode, setKp, etc.)
// - Update myData via setters
// - Trigger eeprom_save
// - Send ACK back to PC

// Error handling:
// - CRC mismatch → NACK
// - Unknown command → NACK
// - All logged via err_output
```
**Depends on:** uart_utils, my_struct, eeprom_save_recovery, err_output
**Used by:** main_app

#### 9️⃣ `eeprom_save_recovery.c/h` - Config Persistence
```c
// Automatic save on each PC command
eeprom_save_config();     // Write my_struct config block to Flash EEPROM
eeprom_load_config();     // Load on startup
bool eeprom_is_valid();   // Check CRC/header

// Handles:
// - EEPROM addressing
// - Wear leveling strategy
// - CRC validation
// - Timeout detection
```
**Depends on:** HAL Flash
**Used by:** pc_comms, main_app (on startup)

#### 🔟 `sd_logg.c/h` - SD Logging
```c
// Asynchronous buffering to SD card
sd_logg_init();
sd_logg_add_sample(voltage, current, temp, mode, state);

// Handles:
// - Circular buffer (fixed size, ~64 rows)
// - Automatic flush when buffer full OR timeout
// - CSV format with timestamps
// - Error handling (SD timeout, full card)

// Folder structure: MMDDHMM/MMDDHMM.csv (date-based)
// Columns: time, setpoint, parameter, current, voltage, DAC, temp1, temp3, ON/OFF, qlmb
```
**Depends on:** FatFS, rtc, err_output
**Used by:** main_app

---

### **FAZA 4 - Inteligencja (Orchestration)**
These modules tie everything together.

#### 1️⃣1️⃣ `regulator.c/h` - PI/PID Control
```c
// 50ms callback from TIM2 interrupt
void regulator_50ms_callback() {
  float error = getSetpoint() - getCurrent();
  float output = regulatorPI(error);
  dac_set_output(output);
}

// Features:
// - Discrete PI/PID algorithm
// - Temperature compensation (bilinear lookup table)
// - Anti-windup for integral term
// - Output saturation (0-15A)
// - Config via my_struct (Kp, Ki, Kd, integral limits)
```
**Depends on:** my_struct, adc_driver, dac_driver, temp_sensor
**Used by:** main_app

#### 1️⃣2️⃣ `system_state.c/h` - State Machine
```c
// Manages overall device state
enum DeviceState { IDLE, MEASURING, REGULATING };

system_state_update(myData);  // Called from main loop
DeviceState state = system_state_get();

// Logic:
// IDLE → MEASURING: GetOnOff() returns true
// MEASURING → REGULATING: All sensors ready
// REGULATING → MEASURING: OnOff becomes false
//
// Safety checks (same state, no transitions):
// - Temp > 170°C → log error, stop regulation
// - Voltage < 11.2V → log error, stop regulation
// - Ah > 250 → log error, stop regulation
//
// Always: Collect data, update LCD, log to SD
```
**Depends on:** my_struct, err_output, temp_sensor, lcd_interface, sd_logg
**Used by:** main_app

#### 1️⃣3️⃣ `main.c` - Orchestrator
```c
// Main event loop (50ms tick from TIM2)
void main() {
    hardware_init();      // Clock, GPIO, SPI, UART, etc.
    modules_init();       // All module init
    eeprom_load_config(); // Restore user settings

    while(1) {
        if (timer_flag) {
            timer_flag = 0;
            
            // Update state machine
            system_state_update(myData);
            
            // Process PC commands
            pc_comms_process();
            
            // Collect sensor data
            adc_read_voltage();
            adc_read_current();
            
            // SD logging (every N cycles)
            sd_logg_add_sample(...);
            
            // LCD update (every 2 seconds)
            lcd_display_periodic();
        }
    }
}

// TIM2 50ms ISR:
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        timer_flag = 1;
        regulator_50ms_callback();  // PI/PID update
        temperature_measurement_periodic();
    }
}
```
**Depends on:** All modules from Faza 1, 2, 3, 4
**Architecture:** Pure orchestration, minimal logic

---

## 🔄 Data Flow
```
┌─────────────────────────────────────┐
│     myData (DataStruct)             │  Central state container
│  setSetpoint/getCurrent/etc.        │
└─────────────────────────────────────┘
         ▲      ▲      ▲      ▲
         │      │      │      │
    ┌────┴┐ ┌──┴──┐ ┌─┴───┐ ┌┴────┐
    │PC   │ │RegΛ │ │LCD  │ │SD   │
    │COMM │ │   │ │ │Displ│ │Log  │
    └─────┘ └─────┘ └─────┘ └─────┘
```

All modules read/write through shared data structure (accessors).
No direct inter-module calls.

---

## ⚙️ Implementation Checklist

### Faza 1: Fundamenty
- [ ] Create directory structure (Core/Inc/modules, Core/Inc/drivers, etc.)
- [ ] my_struct.c/h - Copy existing, add setters
- [ ] uart_utils.c/h - Extract UART init from existing code
- [ ] adc_driver.c/h - Extract MCP3561 logic
- [ ] dac_driver.c/h - Extract DAC control logic

### Faza 2: Independent Modules
- [ ] err_output.c/h - New logger
- [ ] lcd_interface.c/h - Extract from existing interface.c
- [ ] temp_sensor.c/h - Extract DS18B20 logic

### Faza 3: Communication
- [ ] pc_comms.c/h - Extract from komunikacjaPC2.c
- [ ] eeprom_save_recovery.c/h - Extract EEPROM functions
- [ ] sd_logg.c/h - Extract from sd.c

### Faza 4: Integration
- [ ] regulator.c/h - Keep mostly as-is
- [ ] system_state.c/h - New state machine
- [ ] main.c - New orchestrator

---

## 🎯 Key Design Principles

1. **Decoupling:** Modules only communicate via myData accessors
2. **Determinism:** Fixed 50ms loop, predictable timing
3. **Safety:** State machine hides device shutdown logic
4. **Maintainability:** One module = one responsibility
5. **Testability:** Each module can be tested independently
6. **Configurability:** No hardcoded magic numbers

---

## 📝 Next Steps
1. Build directory structure
2. Implement Faza 1 (4 files)
3. Test each Faza 1 module independently
4. Build Faza 2, 3, 4 incrementally
5. Integration testing

**Status:** Ready for implementation
