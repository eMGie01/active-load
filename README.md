# Programmable Active Load for 12 V Batteries (Master’s Thesis)

A full **hardware + firmware** project: a **programmable active electronic load** for testing 12 V batteries.  
Supports three operating modes:

- **CC – Constant Current**  
- **CR – Constant Resistance**  
- **CP – Constant Power**

and continuous operation up to approx. **14 A** discharge current.

The system integrates:

- custom analog power electronics (MOSFET + power resistors),
- precision measurement with **24‑bit ADCs** and a voltage reference,
- **STM32 NUCLEO‑L152RE** as the main controller,
- a **PID regulator with temperature compensation**,
- front‑panel user interface (LCD + encoder + button),
- **SD card logging** (CSV),
- and a **PC application** for remote configuration and monitoring over UART.

---

## Project Overview

The device acts as a **smart load** for 12 V batteries:

1. The tested battery is connected to the load terminals.  
2. The device **draws current** according to the selected mode (CC/CR/CP).  
3. The STM32 continuously measures:
   - battery **voltage**,
   - discharge **current**,
   - **temperatures** (MOSFET + ambient),
4. A **PID + temperature compensation** algorithm sets the MOSFET gate via a DAC so that:
   - the controlled quantity (I/R/P) follows the setpoint,
   - MOSFET behaviour is stable across temperature changes.
5. The device:
   - displays live data and settings on an LCD,
   - logs detailed measurements to SD card,
   - enforces **safety and stop conditions** (voltage, temperature, time, extracted charge),
   - can be monitored/controlled from a **PC GUI**.

This is a complete measurement & control system designed from scratch as a Master’s thesis.

---

## Key Features

- **Three operating modes**
  - **CC (Constant Current)** – keeps discharge current at a configured level (from ~0.15 A up to 14 A).
  - **CR (Constant Resistance)** – emulates a resistor (current changes with voltage, resistance stays constant).
  - **CP (Constant Power)** – adjusts current so that electrical power drawn from the battery remains constant.

- **PID control with temperature compensation**
  - PID works on current (or derived value) with:
    - anti‑windup,
    - proper clamping for safe operation.
  - Temperature compensation uses a precomputed **Vgs(I, T)** characteristic:
    - MOSFET behaviour modelled in LTspice,
    - converted to lookup tables,
    - interpolated in real time to correct DAC output vs. temperature.

- **High‑resolution measurement**
  - Two **MCP3561 24‑bit ADCs** connected over SPI:
    - one for **battery voltage**,
    - one for **current** (via shunt).
  - Precision **2.5 V reference** (ADR441).
  - Calibration routines compensate gain/offset and cable resistance.

- **Thermal management & safety**
  - IRLZ44N MOSFET + power resistors on a heatsink.
  - **Two DS18B20** temperature sensors:
    - MOSFET heatsink,
    - ambient.
  - **Fan** controlled by STM32 (PWM / on-off).
  - Programmable **stop criteria**:
    - minimum battery voltage,
    - maximum MOSFET temperature,
    - maximum test time,
    - maximum extracted charge.

- **Local user interface**
  - **2×16 character LCD**.
  - **Rotary encoder** with push button.
  - Additional front‑panel button.
  - Screens for:
    - live measurements,
    - mode selection (CC/CR/CP),
    - enabling/disabling load,
    - logging configuration.

- **Data logging to SD card**
  - microSD via SPI + FatFS.
  - CSV logs with:
    - timestamps (RTC),
    - mode & setpoints,
    - U, I, computed P and R,
    - MOSFET & ambient temperatures,
    - DAC output,
    - total discharged charge,
    - on/off status.

- **PC application integration**
  - Custom **UART protocol** with framing and checksum.
  - **C# desktop application**:
    - sets configuration (mode, setpoints, limits, PID gains, cable resistance),
    - starts/stops tests,
    - displays key parameters and debug data.

---

## Hardware Summary

*(High-level, without drowning in analog details)*

- **Controller**: STM32 NUCLEO‑L152RE
- **Power stage**:
  - IRLZ44N N‑MOSFET working in the linear region,
  - high‑power resistors (switchable combinations) to offload part of the dissipation.
- **Measurement**:
  - 2 × MCP3561 (24‑bit ADCs) over SPI,
  - ADR441 precision reference,
  - shunt and voltage dividers with op‑amp conditioning.
- **Temperature + cooling**:
  - 2 × DS18B20,
  - 12 V fan driven by STM32.
- **Storage**:
  - microSD card (SPI, FatFS).
- **User I/O**:
  - 2×16 LCD,
  - rotary encoder + push,
  - extra button,
  - status LEDs.
- **External interface**:
  - UART to PC (USB‑UART via NUCLEO).

Full schematics and PCB layout are documented in the thesis (`msc_thesis.pdf`).

---

## Firmware Architecture (STM32)

Firmware is written in C (STM32CubeIDE + HAL) with a **modular design**.

### Core Modules (conceptually)

- **`main.c` / `main.h`**
  - System initialization:
    - clocks, GPIO, SPI, UART, TIM, RTC, SD, ADCs, DAC.
  - Startup configuration:
    - default mode, default limits, logging options.
  - Main control loop:

- **Data structure module – `mystruct.c/.h`**
  - Single **global data model** holding:
    - configuration: mode, setpoints, limits, logging flags,
    - device state: ON/OFF, errors, which stop condition triggered.
  - Provides setter/getter‑like access:
    - used by firmware and PC app to stay in sync.
- **Regulator module – `regulator.c/.h`**
  - Contains **PID controller** + **temperature compensation** logic.
  - Called periodically from a timer interrupt:
    - reads current measurements from data structure,
    - computes new control value (desired current),
    - applies temperature compensation (lookup / interpolation),
    - writes resulting DAC code to control MOSFET gate.

  - Implements LCD screens and encoder/button navigation.
  - Handles:
    - main view (live data),
    - turning load ON/OFF,
    - selecting mode (CC/CR/CP),
    - toggling logging and some configuration parameters.

- **PC communication module – `komunikacjaPC2.c/.h`**
  - Implements protocol over UART:
    - framing (header, length),
    - payload encoding/decoding (floats, ints, enums),
    - checksum.
  - Supports:
    - sending measurement snapshot and status,
    - receiving updated configuration from PC.
  - Uses **circular buffer** and state machine to handle frames robustly.

  - `sd.c/.h` – SD card + FatFS integration, CSV logging.
  - `rtc.c/.h` – timekeeping and timestamps for logs.
  - `spi.c/.h` – SPI configuration for ADCs, DAC, SD.
  - `usart.c/.h` – UART configuration for PC comm.
  - `tim.c/.h` – timers for control loop and time base.
  - `gpio.c/.h` – pin configuration (MOSFET drive, fan, UI, etc.).

- **Support / HAL**
  - `stm32l1xx_hal_msp.c`, `stm32l1xx_it.c`, `system_stm32l1xx.c`, `syscalls.c`, `sysmem.c`, `stm32l1xx_hal_conf.h` – standard HAL support and interrupt handlers.
---

## Control & Runtime Behaviour

1. **Initialization**
   - MCU initializes peripherals (SPI, UART, timers, GPIO, RTC, SD).
   - ADCs and DAC are configured.
   - Data structure loaded with defaults (mode, limits, logging off).

2. **Idle / Configuration**
     - selects mode (CC/CR/CP),
     - sets desired current, resistance or power,
     - configures stop criteria and logging,
     - enables/disables the load.
   - Alternatively, configuration is sent from the PC application over UART.

   - When the load is **ON**:
     - Timer‑based routine periodically:
       - reads new ADC samples (U, I),
       - reads temperatures (DS18B20),
     - Main loop:
       - checks stop conditions (Vmin, Tmax, Qmax, tmax),
       - writes data samples to SD if logging is enabled,
       - updates LCD view,
       - handles incoming PC commands.

4. **Stopping and Logging**
   - On any stop condition or user STOP:
     - load turns OFF (current set to 0),
     - logging file is safely closed,
     - information about which condition stopped the test is saved,
     - device returns to safe idle state.

5. **PC Integration**
   - At any time, the PC app can:
     - read current status and measurements,
     - change configuration and setpoints,
     - start/stop the test,
     - show debug information that is not visible on the LCD (e.g. regulator internals).

---

## Example Use Cases

- **Battery characterization in CC mode**
  - Set constant discharge current (e.g. 1.4 A).
  - Enable logging and set Vmin for the battery type.
  - Run test until voltage reaches the threshold.
  - Use CSV log to compute:
    - discharged capacity [Ah],
    - voltage sag profile,
    - internal resistance trends.

- **Load emulation in CR mode**
  - Emulate a specific resistive load (e.g. 10 Ω).
  - Observe how current changes vs. voltage drop during the test.

- **Lifetime/aging tests in CP mode**
  - Set a constant power (e.g. 50 W).
  - Observe how current ramps up as battery voltage decreases.
  - Use logs to compare multiple cycles over time.

---

## What This Project Shows (for Recruiters)

From an **embedded systems** perspective:

- Experience with the **complete lifecycle** of a serious embedded device:
  - concept → schematics → PCB → firmware → PC tooling → experiments.
- Confidence in:
  - STM32 HAL (SPI, UART, timers, GPIO, RTC),
  - 24‑bit ADCs and precision analog front‑ends,
  - **PID control** in the context of power electronics,
  - SD card logging and file systems (FatFS),
  - robust serial protocols and circular buffer parsing.

From a **software engineering** perspective:

- Modular architecture with clear separation of:
  - measurement,
  - control (regulator),
  - UI,
  - logging,
  - PC communication.
- Consistent data model shared between embedded firmware and PC app (`mystruct`).
- Custom desktop tooling (C#) extending and supporting the embedded device.
- Attention to:
  - safety (stop conditions),
  - robustness (long‑running 17 h+ tests),
  - testability (logging, debug info).

---

## My Contribution

This was my **Master’s thesis**, and I was responsible for the full stack:

- **Hardware**
  - system concept, block diagrams and requirements definition,
  - schematic design and PCB layout,
  - component selection (MOSFET, ADCs, DAC, references, sensors),
  - mechanical integration (heatsink, fan, enclosure, front panel).

- **Firmware (STM32)**
  - designing overall architecture and module boundaries,
  - implementing:
    - PID controller and temperature compensation (`regulator`),
    - data structure module (`mystruct`),
    - PC communication (`komunikacjaPC2`),
    - UI (`interface`),
    - SD logging (`sd`),
    - temperature sensing, measurement, and safety logic.

- **PC Application**
  - C# desktop app:
    - serial protocol implementation,
    - GUI for configuration and visualization.

- **Testing & validation**
  - planning and running experiments (CC/CR/CP),
  - logging and analysing data,
  - verifying accuracy, stability and robustness,
  - documenting results and conclusions in the thesis.

---

## Possible Future Improvements

If I revisit this project, I would consider:

- extending voltage/current range beyond 12 V and 14 A,
- adding real‑time plotting and profile management to the PC app,
- making the device network‑capable (Ethernet/Wi‑Fi) with a web UI,
- designing a second‑generation PCB focused on manufacturability and EMI robustness,
- supporting programmable test sequences (multi‑step profiles: CC → CP → rest → repeat).
