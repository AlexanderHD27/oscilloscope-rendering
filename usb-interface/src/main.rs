use std::{any::Any, time::Duration, usize};

use libusb::{Context, Device};

const USB_PICO_VID: u16 = 0xcafe;
const USB_PICO_PID: u16 = 0xbabe;

pub const CONFIG_INTERFACE: u8 = 1;
pub const MAX_INSTRUCTION_LIST_SIZE: usize = 128;
pub const SAMPLE_SIZE: u16 = 0x1000;

fn find_pico_usb_device<'a>(context: &'a Context) -> Option<Device<'a>> {
    for device in context.devices().unwrap().iter() {
        let device_desc = device.device_descriptor().unwrap();

        if device_desc.vendor_id() == USB_PICO_VID && device_desc.product_id() == USB_PICO_PID {           
            return Some(device);
        }
    }

    None
}

pub enum CHANNEL {
    X = 0x00,
    Y = 0x80
}

pub enum INSTRUCTIONS {
    NONE = 0x00,
    CONST = 0x01,
    LINE = 0x02,
    CUBIC = 0x03,
    QUADRATIC = 0x04,
    SINE = 0x05
}

pub struct InstructionBuilder {
    buffer: [u8; MAX_INSTRUCTION_LIST_SIZE],
    offset: usize
}

// referer to  https://www.mintcalc.com/projects/osci-rendering/docs/md_docs_Instructions.html
mod parameter_mappings {
    use std::usize;

    pub const CONST_LEVEL: usize = 0;

    pub const LINE_FROM: usize = 0;
    pub const LINE_TO: usize   = 1;

    pub const QUADRATIC_FROM: usize = 0;
    pub const QUADRATIC_TO: usize   = 1;
    pub const QUADRATIC_CTRL: usize = 2;

    pub const CUBIC_FROM:      usize = 0;
    pub const CUBIC_CTRL_FROM: usize = 1;
    pub const CUBIC_CTRL_TO:   usize = 2;
    pub const CUBIC_TO:        usize = 3;

    pub const SINE_LOW:    usize = 0;
    pub const SINE_HIGH:   usize = 1;
    pub const SINE_PERIOD: usize = 2;
    pub const SINE_PHASE:  usize = 3;

    pub const INS_LENGTH_NONE:      usize = 3;
    pub const INS_LENGTH_CONST:     usize = 5;
    pub const INS_LENGTH_LINE:      usize = 7;
    pub const INS_LENGTH_CUBIC:     usize = 9;
    pub const INS_LENGTH_QUADRATIC: usize = 11;
    pub const INS_LENGTH_SINE:      usize = 11;
}


impl InstructionBuilder {
    pub fn new() -> InstructionBuilder {
        InstructionBuilder { offset: 0, buffer: [0; MAX_INSTRUCTION_LIST_SIZE] }
    }

    pub fn finish(&self) -> &[u8] {
        return &self.buffer[..self.offset];
    }

    fn add_instruction(&mut self, channel: CHANNEL, length: u16, ins_id: INSTRUCTIONS) {
        self.buffer[self.offset] = (channel as u8) | (ins_id as u8);
        self.buffer[self.offset + 1] = ((length & 0xff00) >> 8) as u8;
        self.buffer[self.offset + 2] = ((length & 0x00ff) >> 0) as u8;
    }

    fn add_parameter(&mut self, param_offset: usize, value: u16) {
        self.buffer[self.offset + param_offset*2 + 3] = ((value & 0xff00) >> 8) as u8;
        self.buffer[self.offset + param_offset*2 + 4] = ((value & 0x00ff) >> 0) as u8;
    }


    pub fn add_none(&mut self, channel: CHANNEL, length: u16) {
        self.add_instruction(channel, length, INSTRUCTIONS::NONE);
        self.offset += parameter_mappings::INS_LENGTH_NONE;
    }

    pub fn add_const(&mut self, channel: CHANNEL, length: u16, level: u16) {
        self.add_instruction(channel, length, INSTRUCTIONS::CONST);
        self.add_parameter(parameter_mappings::CONST_LEVEL, level);
        self.offset += parameter_mappings::INS_LENGTH_CONST;
    }

    pub fn add_line(&mut self, channel: CHANNEL, length: u16, from: u16, to: u16) {
        self.add_instruction(channel, length, INSTRUCTIONS::LINE);
        self.add_parameter(parameter_mappings::LINE_FROM, from);
        self.add_parameter(parameter_mappings::LINE_TO, to);
        self.offset += parameter_mappings::INS_LENGTH_LINE;
    }

    pub fn add_qudratic(&mut self, channel: CHANNEL, length: u16, from: u16, ctrl: u16, to: u16) {
        self.add_instruction(channel, length, INSTRUCTIONS::QUADRATIC);
        self.add_parameter(parameter_mappings::QUADRATIC_FROM, from);
        self.add_parameter(parameter_mappings::QUADRATIC_TO, to);
        self.add_parameter(parameter_mappings::QUADRATIC_CTRL, ctrl);
        self.offset += parameter_mappings::INS_LENGTH_QUADRATIC;
    }

    pub fn add_cubic(&mut self, channel: CHANNEL, length: u16, from: u16, ctrl_from: u16, ctrl_to: u16, to: u16) {
        self.add_instruction(channel, length, INSTRUCTIONS::CUBIC);
        self.add_parameter(parameter_mappings::CUBIC_FROM, from);
        self.add_parameter(parameter_mappings::CUBIC_CTRL_FROM, ctrl_from);
        self.add_parameter(parameter_mappings::CUBIC_CTRL_TO, ctrl_to);
        self.add_parameter(parameter_mappings::CUBIC_TO, to);
        self.offset += parameter_mappings::INS_LENGTH_CUBIC;
    }

    pub fn add_sin(&mut self, channel: CHANNEL, length: u16, low: u16, high: u16, period: u16, phase: u16) {
        self.add_instruction(channel, length, INSTRUCTIONS::SINE);
        self.add_parameter(parameter_mappings::SINE_LOW, low);
        self.add_parameter(parameter_mappings::SINE_HIGH, high);
        self.add_parameter(parameter_mappings::SINE_PERIOD, period);
        self.add_parameter(parameter_mappings::SINE_PHASE, phase);
        self.offset += parameter_mappings::INS_LENGTH_SINE;
    }
}

fn main() {
    let mut context = libusb::Context::new().unwrap();
    context.set_log_level(libusb::LogLevel::Info);

    let device = find_pico_usb_device(&context);

    let mut ins = InstructionBuilder::new();
    ins.add_const(CHANNEL::X, SAMPLE_SIZE/2, 0x0000);
    ins.add_const(CHANNEL::X, SAMPLE_SIZE/2, 0xffff);
    ins.add_const(CHANNEL::Y, SAMPLE_SIZE/2, 0xffff);
    ins.add_const(CHANNEL::Y, SAMPLE_SIZE/2, 0x0000);

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

                    if dev_handle.kernel_driver_active(CONFIG_INTERFACE).unwrap() {
                        dev_handle.detach_kernel_driver(CONFIG_INTERFACE).unwrap();
                    }

                    dev_handle.set_active_configuration(CONFIG_INTERFACE).unwrap();

                    //dev_handle.set_active_configuration(0).unwrap();
                    match dev_handle.claim_interface(CONFIG_INTERFACE) {
                        Err(error) => {
                            println!("Could no claim interface: {}", error);
                        }
                        Ok(_) => ()
                    } 

                    dev_handle.write_bulk(0x02, &mut ins.finish(), Duration::new(10, 0)). unwrap();
                }
                Err(error) => {
                    println!("Error, could not open devices: {}", error);
                }
            } 
        }
    }    
}