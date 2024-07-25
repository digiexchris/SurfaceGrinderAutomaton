# Include Unity framework
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/unity)

# add_subdirectory(unity)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

# MACRO(HEADER_DIRECTORIES return_list)
#     FILE(GLOB_RECURSE new_list *.h*)
#     SET(dir_list "")
#     FOREACH(file_path ${new_list})
#         GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
#         SET(dir_list ${dir_list} ${dir_path})
#     ENDFOREACH()
#     LIST(REMOVE_DUPLICATES dir_list)
#     SET(${return_list} ${dir_list})
# ENDMACRO()

FILE(GLOB_RECURSE app_sources 
#main app
# ./src/*.c*
#tests
./src/*.c*
)

include(FetchContent)

# Fetch CppUTest
FetchContent_Declare(
  cpputest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(cpputest)

add_executable(SurfaceGrinderAtomaton_tests ${app_sources})

# Link with pico_stdlib and other required libraries
target_link_libraries(SurfaceGrinderAtomaton_tests pico_stdlib CppUTest)

# Enable USB output, disable UART output
pico_enable_stdio_usb(SurfaceGrinderAtomaton_tests 0)
pico_enable_stdio_uart(SurfaceGrinderAtomaton_tests 1)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(SurfaceGrinderAtomaton_tests)

if (WIN32)
add_custom_command(TARGET SurfaceGrinderAtomaton POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded! "
 COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/win64/flash.bat $<TARGET_FILE:SurfaceGrinderAtomaton_tests>
)
else()

if(NOT BUILD_TESTS)
add_custom_command(TARGET SurfaceGrinderAtomaton POST_BUILD
COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded!"
COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/linux/flash.sh $<TARGET_FILE:SurfaceGrinderAtomaton_tests>
)

endif()

endif()

add_dependencies(SurfaceGrinderAtomaton_tests copy_compile_commands)

add_dependencies(SurfaceGrinderAtomaton_tests SurfaceGrinderAtomaton)