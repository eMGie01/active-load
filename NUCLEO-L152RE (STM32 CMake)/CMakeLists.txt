cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME active-load)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_SOURCE_DIR}/Modules/app/src/app_init.c
    ${CMAKE_SOURCE_DIR}/Modules/comms/src/comms_pc.c
    ${CMAKE_SOURCE_DIR}/Modules/control/src/load_output.c
    ${CMAKE_SOURCE_DIR}/Modules/control/src/regulator.c
    ${CMAKE_SOURCE_DIR}/Modules/control/src/safety.c
    ${CMAKE_SOURCE_DIR}/Modules/logger/src/logger_sd.c
    ${CMAKE_SOURCE_DIR}/Modules/logger/src/logger_uart.c
    ${CMAKE_SOURCE_DIR}/Modules/measurement/src/measurement.c
    ${CMAKE_SOURCE_DIR}/Modules/measurement/src/adc_current_config.c
    ${CMAKE_SOURCE_DIR}/Modules/measurement/src/adc_voltage_config.c
    ${CMAKE_SOURCE_DIR}/Modules/measurement/src/temp_sensor.c
    ${CMAKE_SOURCE_DIR}/Modules/param_table/src/param_table.c
    ${CMAKE_SOURCE_DIR}/Modules/state/src/app_state.c
    ${CMAKE_SOURCE_DIR}/Modules/storage/src/config_store.c
    ${CMAKE_SOURCE_DIR}/Modules/ui/src/lcd_ui.c

    ${CMAKE_SOURCE_DIR}/AdDrivers/MCP3561T/src/mcp3561t_init.c
    ${CMAKE_SOURCE_DIR}/AdDrivers/MCP3561T/src/mcp3561t_runtime.c
    ${CMAKE_SOURCE_DIR}/AdDrivers/DAC8830/src/dac8830.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_SOURCE_DIR}/Modules/app/src
    ${CMAKE_SOURCE_DIR}/Modules/comms/src
    ${CMAKE_SOURCE_DIR}/Modules/control/src
    ${CMAKE_SOURCE_DIR}/Modules/logger/src
    ${CMAKE_SOURCE_DIR}/Modules/measurement/src
    ${CMAKE_SOURCE_DIR}/Modules/param_table/src
    ${CMAKE_SOURCE_DIR}/Modules/state/src
    ${CMAKE_SOURCE_DIR}/Modules/storage/src
    ${CMAKE_SOURCE_DIR}/Modules/ui/src
    
    ${CMAKE_SOURCE_DIR}/AdDrivers/MCP3561T/src
    ${CMAKE_SOURCE_DIR}/AdDrivers/DAC8830/src
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
