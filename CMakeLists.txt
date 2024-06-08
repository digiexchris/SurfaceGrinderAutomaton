# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.h*)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

add_library(freertos_config INTERFACE)

target_include_directories(freertos_config SYSTEM
INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src
)

target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)

set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)

set(FREERTOS_PORT "GCC_ARM_CM0" CACHE STRING "" FORCE)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
   set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
# set(PICO_SDK_PATH ${USERHOME}/.pico-sdk/sdk/1.5.1)
# set(PICO_TOOLCHAIN_PATH ${USERHOME}/.pico-sdk/toolchain/13_2_Rel1)
if(WIN32)
    set(pico-sdk-tools_DIR ${USERHOME}/.pico-sdk/tools/1.5.1)
    include(${pico-sdk-tools_DIR}/pico-sdk-tools-config.cmake)
    include(${pico-sdk-tools_DIR}/pico-sdk-tools-config-version.cmake)
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(SurfaceGrinderAtomaton C CXX ASM)

include(FetchContent)

FetchContent_Declare( freertos
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG        V11.1.0 #Note: Best practice to use specific git-hash or tagged version
)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
FILE(GLOB_RECURSE app_sources 
#main app
./src/*.c*
#tests
# ./test/*.c*
)

list(LENGTH header_dir_list header_dir_list_count)
message(STATUS "[INFO] Found ${header_dir_list_count} header directories.")

# target_include_directories(
#     my_program
#     PUBLIC
#     ${header_dir_list} # Recursive
# )

add_executable(SurfaceGrinderAtomaton ${app_sources} )

pico_set_program_name(SurfaceGrinderAtomaton "SurfaceGrinderAtomaton")
pico_set_program_version(SurfaceGrinderAtomaton "0.1")

# Generate PIO header
pico_generate_pio_header(SurfaceGrinderAtomaton ${CMAKE_CURRENT_LIST_DIR}/pio/blink.pio)
pico_generate_pio_header(SurfaceGrinderAtomaton ${CMAKE_CURRENT_LIST_DIR}/pio/stepper.pio)

# Modify the below lines to enable/disable output over UART/USB
# pico_enable_stdio_uart(SurfaceGrinderAtomaton 0)
# pico_enable_stdio_usb(SurfaceGrinderAtomaton 1)


# Add the standard library to the build


# Add the standard include files to the build
target_include_directories(SurfaceGrinderAtomaton PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${header_dir_list}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

target_compile_definitions(freertos_config INTERFACE ${definitions})
target_compile_options(freertos_config INTERFACE ${options})
FetchContent_MakeAvailable(freertos)
message(STATUS "Adding FreeRTOS includes")

message(STATUS "Header directory: ${freertos_SOURCE_DIR}")

target_include_directories(SurfaceGrinderAtomaton PRIVATE
    ${freertos_SOURCE_DIR}/include
    ${freertos_SOURCE_DIR}/portable/GCC/ARM_CM0
)

# Add FreeRTOS as a library
add_library(FreeRTOS STATIC
    ${freertos_SOURCE_DIR}/event_groups.c
    ${freertos_SOURCE_DIR}/list.c
    ${freertos_SOURCE_DIR}/queue.c
    ${freertos_SOURCE_DIR}/stream_buffer.c
    ${freertos_SOURCE_DIR}/tasks.c
    ${freertos_SOURCE_DIR}/timers.c
    ${freertos_SOURCE_DIR}/croutine.c
    ${freertos_SOURCE_DIR}/event_groups.c
    ${freertos_SOURCE_DIR}/portable/MemMang/heap_3.c
    ${freertos_SOURCE_DIR}/portable/GCC/ARM_CM0/port.c
    ${freertos_SOURCE_DIR}/portable/GCC/ARM_CM0/portasm.c
    ${freertos_SOURCE_DIR}/portable/Common/mpu_wrappers_v2.c
    ${freertos_SOURCE_DIR}/portable/GCC/ARM_CM0/mpu_wrappers_v2_asm.c
)

# Build FreeRTOS
target_include_directories(FreeRTOS PUBLIC
${CMAKE_CURRENT_LIST_DIR}/src
    ${freertos_SOURCE_DIR}/include
    ${freertos_SOURCE_DIR}/portable/GCC/ARM_CM0
)

target_link_libraries(SurfaceGrinderAtomaton
        pico_stdlib
        hardware_pio
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        freertos_config
        FreeRTOS
        )

pico_add_extra_outputs(SurfaceGrinderAtomaton)

