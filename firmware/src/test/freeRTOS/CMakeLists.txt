project(freeRTOS_test)

add_executable(freeRTOS_test
    main.c
)

# Add freeRTOS Config Header
target_include_directories(FreeRTOS PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in dependencies
target_link_libraries(freeRTOS_test 
    pico_stdlib
    hardware_gpio
    FreeRTOS
)

# create map/bin/hex file etc.
pico_add_extra_outputs(freeRTOS_test)

# add url via pico_set_program_url
example_auto_set_url(freeRTOS_test)