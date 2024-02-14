project(usb_test)

add_executable(usb_test
    main.c
    usb_descriptors.c
)

# Add tiny USB config header
target_include_directories(usb_test PUBLIC PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# pull in dependencies
target_link_libraries(usb_test 
    pico_stdlib
    pico_unique_id 
    tinyusb_device tinyusb_board
)

# create map/bin/hex file etc.
pico_add_extra_outputs(usb_test)

# add url via pico_set_program_url
example_auto_set_url(usb_test)