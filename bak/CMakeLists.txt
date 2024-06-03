cmake_minimum_required(VERSION 3.13.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(pico_sdk_import.cmake)


include(pico_extras_import_optional.cmake)



if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)
# Add blink example
add_subdirectory(blink)

# Add hello world example
add_subdirectory(hello_world)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

project(app LANGUAGES C CXX ASM)

FILE(GLOB_RECURSE app_sources 
#main app
./src/*.c*
#tests
# ./test/*.c*
)
include(FetchContent)

set(LV_USE_ILI9341  1)
set(LV_CONF_BUILD_DISABLE_DEMOS 1)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL 1)
set(LV_CONF_BUILD_DISABLE_EXAMPLES 1)
# set(LV_CONF_PATH src/lv_conf.h)
set(LV_CONF_INCLUDE_SIMPLE 1)


if (TARGET tinyusb_device)
    add_executable(app
    ${app_sources}
            )

    # pull in common dependencies
    target_link_libraries(app pico_stdlib)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(app 1)
    pico_enable_stdio_uart(app 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(app)

    # add url via pico_set_program_url
    example_auto_set_url(app)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()