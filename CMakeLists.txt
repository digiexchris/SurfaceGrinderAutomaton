cmake_minimum_required(VERSION 3.13)

#if(WIN32)
#add_custom_target(RunEnvScript
#    COMMAND ${CMAKE_COMMAND} -E env call ${PICO_INSTALL_DIR}/pico-env.cmd
##    WORKING_DIRECTORY ${PICO_INSTALL_DIR}
 #   COMMENT "Setting environment variables"
#)
#endif()


# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
project(SurfaceGrinderAtomaton C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

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

# target_compile_definitions(FreeRTOS-Kernel INTERFACE -Wno-unused-variable)
#header_directories(header_dir_list)

FILE(GLOB_RECURSE app_sources 
#main app
./src/*.c*
#tests
# ./test/*.c*
)

pico_sdk_init()

add_executable(SurfaceGrinderAtomaton
${app_sources}
        )

# target_compile_definitions(SurfaceGrinderAtomaton PRIVATE
#         mainCREATE_SIMPLE_BLINKY_DEMO_ONLY=1
#         )
# Set global compiler options
add_compile_options(
    ### Gnu/Clang C Options
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>

    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-variable>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-parameter>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-function>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-implicit-function-declaration>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-type-limits>
)

# # Override compiler options for SurfaceGrinderAtomaton
# target_compile_options(SurfaceGrinderAtomaton PRIVATE
#     ### Gnu/Clang C Options
#     $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
#     $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>

#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
#     $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
#     # Remove the -Wno-unused-variable flag for this target
# )

 if (WIN32)
        add_custom_command(TARGET SurfaceGrinderAtomaton POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded! Skipping auto flash"
		)
    else()

    add_custom_command(TARGET SurfaceGrinderAtomaton POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded!"
       COMMAND /bin/bash ${CMAKE_CURRENT_LIST_DIR}/flash.sh
   
   )
endif()

target_include_directories(SurfaceGrinderAtomaton PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${header_dir_list}
        ${CMAKE_CURRENT_LIST_DIR}/src)


# file(TO_NATIVE_PATH "${TOP}/hw/bsp/${FAMILY}/boards/${BOARD}/board.cmake" board_cmake)

# if(EXISTS ${board_cmake})
#     include(${board_cmake})
# endif()

pico_generate_pio_header(SurfaceGrinderAtomaton ${CMAKE_CURRENT_LIST_DIR}/pio/blink.pio)
pico_generate_pio_header(SurfaceGrinderAtomaton ${CMAKE_CURRENT_LIST_DIR}/pio/stepper.pio)

# Create an interface target to set system include directories for Pico SDK libraries
add_library(pico_system_include_dirs INTERFACE)
target_include_directories(pico_system_include_dirs SYSTEM INTERFACE ${PICO_INCLUDE_DIRS})

# Include directories for other Pico SDK libraries
target_link_libraries(pico_system_include_dirs INTERFACE pico_stdlib hardware_pio pico_printf
)


target_link_libraries(SurfaceGrinderAtomaton pico_system_include_dirs FreeRTOS-Kernel FreeRTOS-Kernel-Heap4)
pico_add_extra_outputs(SurfaceGrinderAtomaton)

# Use USB uart
pico_enable_stdio_usb(SurfaceGrinderAtomaton 0)
pico_enable_stdio_uart(SurfaceGrinderAtomaton 1)
