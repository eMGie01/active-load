set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

set(STM32CUBE_GNU_TOOLS_ROOT "$ENV{HOME}/.local/share/stm32cube/bundles/gnu-tools-for-stm32/14.3.1+st.2")
set(STM32CUBE_GNU_TOOLS_BIN "${STM32CUBE_GNU_TOOLS_ROOT}/bin")

find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc HINTS "${STM32CUBE_GNU_TOOLS_BIN}")
find_program(ARM_NONE_EABI_GXX arm-none-eabi-g++ HINTS "${STM32CUBE_GNU_TOOLS_BIN}")
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy HINTS "${STM32CUBE_GNU_TOOLS_BIN}")
find_program(ARM_NONE_EABI_SIZE arm-none-eabi-size HINTS "${STM32CUBE_GNU_TOOLS_BIN}")

if(NOT ARM_NONE_EABI_GCC OR NOT ARM_NONE_EABI_GXX)
    message(FATAL_ERROR "arm-none-eabi GCC toolchain was not found. Install STM32CubeCLT/GNU Tools for STM32 or add arm-none-eabi-* to PATH.")
endif()

set(CMAKE_C_COMPILER                ${ARM_NONE_EABI_GCC})
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${ARM_NONE_EABI_GXX})
set(CMAKE_LINKER                    ${ARM_NONE_EABI_GXX})
set(CMAKE_OBJCOPY                   ${ARM_NONE_EABI_OBJCOPY})
set(CMAKE_SIZE                      ${ARM_NONE_EABI_SIZE})

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU specific flags
set(TARGET_FLAGS "-mcpu=cortex-m3 ")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32L152XX_FLASH.ld\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
set(TOOLCHAIN_LINK_LIBRARIES "m")
