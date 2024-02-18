use std::{any::Any, time::Duration};

use libusb::{Context, Device};

const USB_PICO_VID: u16 = 0xcafe;
const USB_PICO_PID: u16 = 0xbabe;

fn find_pico_usb_device<'a>(context: &'a Context) -> Option<Device<'a>> {
    for device in context.devices().unwrap().iter() {
        let device_desc = device.device_descriptor().unwrap();

        if device_desc.vendor_id() == USB_PICO_VID && device_desc.product_id() == USB_PICO_PID {           
            return Some(device);
        }
    }

    None
}

fn main() {
    let mut context = libusb::Context::new().unwrap();
    context.set_log_level(libusb::LogLevel::Debug);

    let device = find_pico_usb_device(&context);

    match device {
        None => println!("No Vector Display found :/"),
        Some(dev) => {
            println!("Found Bus {:03} Device {:03} Speed: {:?}",
                dev.bus_number(),
                dev.address(),
                dev.speed()
            );

            match dev.open() {
                Ok(mut dev_handle) => {
                    let mut buf: [u8; 64] = [0xaa; 64];

                    if dev_handle.kernel_driver_active(1).unwrap() {
                        dev_handle.detach_kernel_driver(1).unwrap();
                    }

                    dev_handle.set_active_configuration(1).unwrap();

                    //dev_handle.set_active_configuration(0).unwrap();
                    match dev_handle.claim_interface(1) {
                        Err(error) => {
                            println!("Could no claim interface: {}", error);
                        }
                        Ok(_) => ()
                    } 


                    dev_handle.write_bulk(0x02, &mut buf, Duration::new(10, 0)). unwrap();
                    println!("{:?}", buf);
                }
                Err(error) => {
                    println!("Error, could not open devices: {}", error);
                }
            } 
        }
    }    
}