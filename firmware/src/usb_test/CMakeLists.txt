
project(usb_test)
add_executable(usb_test
    main.c
)

# pull in common dependencies
target_link_libraries(usb_test 
    pico_stdlib
)

# create map/bin/hex file etc.
pico_add_extra_outputs(usb_test)

# add url via pico_set_program_url
example_auto_set_url(usb_test)