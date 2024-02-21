use std::time::Duration;
use libusb::{Context, Device, DeviceHandle};

use crate::gen;

pub const DATA_INTERFACE: u8 = 1;
pub const INT_INTERFACE: u8 = 2;

const USB_PICO_VID: u16 = 0xcafe;
const USB_PICO_PID: u16 = 0xbabe;

pub fn find_pico_usb_device<'a>(context: &'a Context) -> Option<Device<'a>> {
    for device in context.devices().unwrap().iter() {
        let device_desc = device.device_descriptor().unwrap();

        if device_desc.vendor_id() == USB_PICO_VID && device_desc.product_id() == USB_PICO_PID {           
            return Some(device);
        }
    }

    None
}

pub struct USBVectorInterface<'a> {
    dev_handle: DeviceHandle<'a>,
    transmission_threshold: usize,
    buffered: usize,
    instruction_buffer: Vec<gen::InstructionBuilder>
}

impl<'a> USBVectorInterface<'a> {
    pub fn new(dev_handle: DeviceHandle<'a>) -> Self {
        Self { 
            dev_handle, 
            transmission_threshold: 64,
            buffered: 0,
            instruction_buffer: Vec::new()
        }
    }

    pub fn submit_instruction(&mut self, ins: gen::InstructionBuilder, timeout: Duration) {

        //if self.buffered + ins.offset + 1 >= self.transmission_threshold {
        //    // We can transmit it all
        //    self.flush(timeout);
        //}

        //self.buffered += ins.offset + 1;
        //self.instruction_buffer.push(ins);
        
        let mut int_buf: [u8; 8] = [0; 8];
        self.dev_handle.write_bulk(0x02, &mut ins.finish(), timeout). unwrap();
        //self.dev_handle.read_interrupt(0x83, &mut int_buf, timeout).unwrap();
    }

    pub fn flush(&mut self, timeout: Duration) {
        let mut transmit_buffer: Vec<u8> = Vec::new();
        transmit_buffer.resize(self.buffered, 0);

        // Copy everything into the transmission buffer
        let mut offset = 0;
        for ins in &self.instruction_buffer {
            transmit_buffer[offset] = ins.get_size() as u8;
            for (i, byte) in ins.finish().iter().enumerate() {
                transmit_buffer[offset + i + 1] = *byte;
            }
            offset += ins.get_size() + 1;
        }

        // Send it off
        let mut int_buf: [u8; 8] = [0; 8];
        self.dev_handle.write_bulk(0x02, &mut transmit_buffer, timeout). unwrap();
        self.dev_handle.read_interrupt(0x83, &mut int_buf, Duration::new(3, 0)).unwrap();

        self.buffered = 0;
        self.instruction_buffer.clear();
    }
}
