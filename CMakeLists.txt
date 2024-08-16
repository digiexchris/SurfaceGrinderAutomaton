cmake_minimum_required(VERSION 3.13)
include(${CMAKE_HOME_DIRECTORY}/pico_sdk_import.cmake)

set(PICO_BOARD pico CACHE STRING "Board type")
project(SurfaceGrinderAtomaton C CXX ASM)
pico_sdk_init()

add_compile_options(
    ### Gnu/Clang C Options
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>
)

# Create an interface target to set system include directories for Pico SDK libraries
add_library(pico_system_include_dirs INTERFACE)
target_include_directories(pico_system_include_dirs SYSTEM INTERFACE ${PICO_INCLUDE_DIRS})

# Include directories for other Pico SDK libraries
# target_link_libraries(pico_system_include_dirs INTERFACE pico_stdlib hardware_pio hardware_gpio pico_multicore pico_stdio_usb pico_printf pico_async_context_threadsafe_background
# )

target_link_libraries(pico_system_include_dirs INTERFACE pico_stdlib hardware_pio hardware_gpio pico_multicore pico_printf pico_async_context_threadsafe_background
)

# Add subdirectories
add_subdirectory(src)

# Optionally add tests
option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to project root"
)

# Create a custom target that always runs the above command
add_custom_target(copy_compile_commands ALL
  DEPENDS ${CMAKE_SOURCE_DIR}/compile_commands.json
)

