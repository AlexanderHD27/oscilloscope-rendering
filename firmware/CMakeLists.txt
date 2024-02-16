cmake_minimum_required(VERSION 3.12)

# Including the SDK (must be before project)

if (NOT "$ENV{PICO_SDK_PATH_OVERRIDE}" STREQUAL "")
    SET(PICO_SDK_PATH "$ENV{PICO_SDK_PATH_OVERRIDE}")
else()
    SET(PICO_SDK_PATH "/home/alexander/Projects/oscilloscope-rendering/firmware/lib/pico-sdk")
endif()

SET(PICO_TOOLCHAIN_PATH $PICO_SDK_PATH/cmake/preload/toolchains/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin>")

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

#include(pico_extras_import_optional.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# adding sources files & includes
add_executable(main
    src/main.c

    src/gen/base.c
    src/gen/pregen.c
    src/gen/gen.c
    src/gen/util.c

    src/dac/driver.c
    src/dac/interface.c
    src/dac/globals.c
    src/dac/processing.c
)

target_include_directories(main PUBLIC 
    include/
)

# Add FreeRTOS
include(freeRTOS.cmake)

target_include_directories(FreeRTOS PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# Adding PIO
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/src/dac/dac.pio)

# pull in common dependencies
target_link_libraries(main 
    pico_stdlib 
    pico_multicore
    hardware_pio
    hardware_dma
    hardware_timer
    FreeRTOS
)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)

# add url via pico_set_program_url
example_auto_set_url(main)

add_subdirectory(src/test/usb)
add_subdirectory(src/test/freeRTOS)
