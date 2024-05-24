cmake_minimum_required(VERSION 3.13.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

# list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(NO_BUILD_TYPE_WARNING TRUE)
if(ELS_TESTS)
set(BOARD native_sim)
add_compile_definitions(ELS_TESTS)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)




find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

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

# FetchContent_Declare(
#     lvgl
#     GIT_REPOSITORY https://github.com/lvgl/lvgl.git
#     GIT_TAG v9.1.0
# )

# FetchContent_MakeAvailable(lvgl)

# include_directories(${LVGL_ROOT_DIR}/src)
include_directories(src)

# MACRO(HEADER_DIRECTORIES return_list)
#     FILE(GLOB_RECURSE new_list lib/lvgl/*.h)
#     SET(dir_list "")
#     FOREACH(file_path ${new_list})
#         GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
#         SET(dir_list ${dir_list} ${dir_path})
#     ENDFOREACH()
#     LIST(REMOVE_DUPLICATES dir_list)
#     SET(${return_list} ${dir_list})
# ENDMACRO()

# HEADER_DIRECTORIES(header_list)

# include_directories(${header_list})

# include_directories(lib/lvgl)

# target_link_libraries(app PUBLIC lvgl)

if(BUILD_TESTS)
    set(NO_BUILD_TYPE_WARNING TRUE)
    set(CMAKE_C_FLAGS -m32)
    set(CMAKE_CXX_FLAGS -m32)

    include(FetchContent)
        FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    FetchContent_Declare(
        trompeloeil
        GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
        GIT_TAG        v47 # or a later release
    )
    FetchContent_MakeAvailable(trompeloeil)

    # include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
    # project(app LANGUAGES C CXX ASM)
    FILE(GLOB_RECURSE test_sources ./test/*.c*)
    include_directories(./test/)
    
endif()


target_sources(app PRIVATE ${app_sources})

if(BUILD_TESTS)
    enable_testing()

    target_link_libraries(
    app
    PUBLIC
    GTest::gtest_main
    )

    add_executable(tests ${app_sources} ${test_sources}) 
    target_link_libraries(
    tests
    PUBLIC
    trompeloeil::trompeloeil
    GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(tests)

endif()