cmake_minimum_required(VERSION 3.13)



# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
project(SurfaceGrinderAtomaton C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(FREERTOS_KERNEL_RP2040_RELATIVE_PATH "portable/ThirdParty/GCC/RP2040")
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
set(PICO_DEOPTIMIZED_DEBUG 1)
set(FREERTOS_PORT "GCC_RP2040" CACHE STRING "" FORCE)
set(PICO_BOARD pico CACHE STRING "Board type")

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel)
message( STATUS "FREERTOS_KERNEL_PATH: ${FREERTOS_KERNEL_PATH}" )

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Pull in FreeRTOS
include(FreeRTOS_Kernel_import.cmake)

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

FILE(GLOB_RECURSE app_sources 
#main app
./src/*.c*
#tests
# ./test/*.c*
)

pico_sdk_init()


# Use USB uart
# pico_enable_stdio_usb(SurfaceGrinderAtomaton 1)
# pico_enable_stdio_uart(SurfaceGrinderAtomaton 1)

add_executable(SurfaceGrinderAtomaton
${app_sources}
        )

# target_compile_definitions(SurfaceGrinderAtomaton PRIVATE
#         mainCREATE_SIMPLE_BLINKY_DEMO_ONLY=1
#         )

target_compile_options( SurfaceGrinderAtomaton PUBLIC
        ### Gnu/Clang C Options
        $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
        $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>

        $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
        $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
        $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
        $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
        )

target_include_directories(SurfaceGrinderAtomaton PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${header_dir_list}
        ${CMAKE_CURRENT_LIST_DIR}/src)

pico_generate_pio_header(SurfaceGrinderAtomaton ${CMAKE_CURRENT_LIST_DIR}/pio/blink.pio)
pico_generate_pio_header(SurfaceGrinderAtomaton ${CMAKE_CURRENT_LIST_DIR}/pio/stepper.pio)


target_link_libraries(SurfaceGrinderAtomaton pico_stdlib hardware_pio FreeRTOS-Kernel FreeRTOS-Kernel-Heap1)
pico_add_extra_outputs(SurfaceGrinderAtomaton)

# Use USB uart
# pico_enable_stdio_usb(SurfaceGrinderAtomaton 1)
# pico_enable_stdio_uart(SurfaceGrinderAtomaton 1)
