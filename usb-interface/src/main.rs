use std::{f32::consts::PI, thread::sleep, time::{Duration, Instant}};

use libusb::{Context, Device, DeviceHandle};

mod gen;
mod usb;

const BUFFER_SIZE: usize = 0x1000;

fn main() {
    let mut context = libusb::Context::new().unwrap();
    context.set_log_level(libusb::LogLevel::Info);

    let mut dev = usb::find_pico_usb_device(&context).unwrap();

    println!("Found Bus {:03} Device {:03} Speed: {:?}",
        dev.bus_number(),
        dev.address(),
        dev.speed()
    );
    
    let mut dev_handle: DeviceHandle = dev.open().unwrap();

    dev_handle.reset().unwrap();
    
    if dev_handle.kernel_driver_active(usb::DATA_INTERFACE).unwrap() {
        dev_handle.detach_kernel_driver(usb::DATA_INTERFACE).unwrap();
    }
    
    if dev_handle.kernel_driver_active(usb::INT_INTERFACE).unwrap() {
        dev_handle.detach_kernel_driver(usb::INT_INTERFACE).unwrap();
    }
    
    dev_handle.claim_interface(usb::DATA_INTERFACE).unwrap();
    dev_handle.claim_interface(usb::INT_INTERFACE).unwrap();

    let mut buf: [u8; BUFFER_SIZE*2*2] = [0; BUFFER_SIZE*2*2];
    let timeout = Duration::from_millis(1000);

    for i in 0..BUFFER_SIZE {
        let x: f32 = (i as f32)/(BUFFER_SIZE as f32);
        //let y = (((f32::sin(x*PI*2.0))*0.5 + 0.5) * (0xffff as f32)) as u16;
        let y = (x*(0xffff as f32)) as u16;

        buf[i*4 + 0] = ((y >> 8) & 0xff) as u8;
        buf[i*4 + 1] = ((y >> 0) & 0xff) as u8;
        buf[i*4 + 2] = ((y >> 8) & 0xff) as u8;
        buf[i*4 + 3] = ((y >> 0) & 0xff) as u8;
    }
    
    for i in 0..((BUFFER_SIZE*4/64)) {
        dev_handle.write_bulk(0x02, &buf[i*64..(i+1)*64], timeout).unwrap();
        //sleep(Duration::from_millis(1/60));
    }
    
}