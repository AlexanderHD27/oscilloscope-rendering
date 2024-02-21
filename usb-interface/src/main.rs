use std::{thread::sleep, time::{Duration, Instant}};

use libusb::{Context, Device, DeviceHandle};

mod usb;
mod gen;

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
    
    let mut usb_interface: usb::USBVectorInterface = usb::USBVectorInterface::new(dev_handle);
    let timeout = Duration::new(1, 0);

    let total_start: Instant = Instant::now();
    for i in 0..0xff {
        let start: Instant = Instant::now();
        let mut ins = gen::InstructionBuilder::new();
        ins.add_const(gen::CHANNEL::Y, gen::SAMPLE_SIZE/2, (0xff - i) << 8);
        ins.add_const(gen::CHANNEL::X, gen::SAMPLE_SIZE/2, i << 8);
        ins.add_sin(gen::CHANNEL::Y, gen::SAMPLE_SIZE/2, 0x0000, 0x7fff, gen::SAMPLE_SIZE/2, 0x0400);
        ins.add_sin(gen::CHANNEL::X, gen::SAMPLE_SIZE/2, 0x0000, 0x7fff, gen::SAMPLE_SIZE/2, 0x0400);                    
        
        usb_interface.submit_instruction(ins, timeout);
        sleep(Duration::from_millis(1));
        
        println!("{} {}ms", i, start.elapsed().as_millis());
    }
    let duration = total_start.elapsed();
    println!("Timer per sample: {}", duration.as_millis()/256);

    usb_interface.flush(timeout);
    println!("Done");

}