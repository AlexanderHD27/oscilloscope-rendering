Firmware for the RPi Pico for Oscilloscope Renderer


[Github](https://github.com/AlexanderHD27/oscilloscope-drawing)

## Downloads
[main.elf](https://www.mintcalc.com/projects/osci-rendering/bin/main.elf)

## Introduction
The Firmware handles data and pushes it to the DACs resulting in a signal for the oscilloscope to be display on.

The RPi Pico is connected via USB to a host machine which provide the image. The Pico can run in two modes: Raw (Signal Data is generated on the host and just passed through) amd Instruction (The Pico recv instructions to generated the signal on the fly and the outputs it)

I'm using an RP2040 on a [Raspberry Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/). Code is written in C/C++ with the [PicoSDK](https://www.raspberrypi.com/documentation/pico-sdk/)with [FreeRTOS](https://www.freertos.org/).
One of the [rp2040](https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html)'s PIO cores is used in combination with a DMA-Channel to archive higher data rate.

Documentation is provided via doxygen and can be found [here](https://www.mintcalc.com/projects/osci-rendering/docs/index.html)

## Development Environment.
This Project is developed with VSCode

### Setup
The VSCode project comes pre-configured is self contained (FreeRTOS, PicoSDK).
But the appropriate tools like gcc, gdb and openocd need to be installed.
You could also use docker to build the project

#### Docker
For this you only need docker to be installed

Building the container
```sh
docker build -t rpi_pico_build ./firmware/docker
```

Running the docs: firmware
```sh
docker run -v "./":/data --rm rpi_pico_build "/data/firmware/docker/build_docs.sh"
```

Running the build: firmware
```sh
docker run -v "./":/data --rm rpi_pico_build "/data/firmware/docker/build_firmware.sh"
```


#### Installing the Build tools
```sh
sudo apt install build-essential cmake gcc-arm-none-eabi gdb-multiarch
```
(install command for ubuntu or debian based distributions)

(I once had an issue with debug in gdb-multiarch, which seems to gone now. But if you have any problems it helps building gdb-multiarch from sources. I'm using v12.2)

#### Installing OpenOCD
You also need to install `openocd` which is done by compiling from sources:
```sh
git clone https://github.com/raspberrypi/openocd.git
cd openocd 
./bootstrap 
./configure --enable-picoprobe
make -j4
sudo make install
```

For Syntax-highlight and debugging you need to load the `rpiPico-code-profile`.

Debugging and Programming is done via a SWD, I used a [Raspberry Pi Debug Probe](https://www.raspberrypi.com/products/debug-probe/) to do so. 
But a [Pico Probe](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf#%5B%7B%22num%22%3A64%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C115%2C841.89%2Cnull%5D) 
or any other SWD-Debugger should work to. You just need to edit the `firmware/scripts/launch_openocd.sh` to fit your debugger.

### Usage
Building is done with the `CMake: build main` VSCode-task

Flashing can be USB: Just press the BOOTSEL Button on the Pico and connect the RPi-Pico via USB. It should now show up as a USB-drive. 
You can now move the .uf2 file form the firmware/build/bin folder. The Pico should now restart and run the uploaded binary.

OR

Flashing can be one via SWD (recommended). You need to connect your Debugger to debug pins of the Pico (see [pinout](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html#the-debug-probe)). Then you need to start openocd via the `Pico: launch openocd` VSCode-Task. The Pico can now be flashed with the `Pico: flash main`-Task (This task automatically builds
everything before flashing)

## Data/Buffer Flow
The Data/Buffer Flow is concert how buffer move with the RP2040, 
without the need of dynamic allocation. The Flow is based FreeRTOS's Queues.

![Diagram of Data Flow](docs/drawio/DataFlow_RP2040/DataFlow_RP2040.drawio.webp)


There are two type of buffers: 
- `frameBuffer_t`: These hold samples that are sent to DACs
- `instructionBuffer_t`: These hold instruction that are used to generate a signal

All buffers are preallocated and are only passed around by pointer, because they are to large (around 8kB in case of the frame_buffer) to be copied. `frameBuffer_t`/`instructionBuffer_t` struct only holds information of about the size and a pointer to the frame/instruction buffer.

The Systems main buffer-cycles:

### 1. instructionBuffer_t Cycle (in Blue)

The IO-handler (e.g USB or UART handler) requests a `instructionBuffer_t` via `dac_acquireInstructionBufferPointer()` from the `g_unusedInstructionBufferQueue` and submits the fill `instructionBuffer_t` via `dac_submitInstructions()` to the `g_instructionBufferQueue`. 

The `gen_processingTaskFunction()` takes one `instructionBuffer_t`r (if available) and generates the output signal with this. The now used `instructionBuffer_t` is submitted to `g_unusedInstructionBufferQueue` for the IO-handler to be picket up and filled again

The Structure/Format of a instruction is documented [here](../docs/Instructions.md)

### 2. frameBuffer_t Cycle (in Yellow)

The `gen_processingTaskFunction()` (aka the Signal Generator) request one unused `frameBuffer_t` from the `g_unusedFrameBufferQueue`. The Signal Generator now fills this `frameBuffer_t` with the signal generated by the instruction it got. This fill buffer is then submitted to the `g_frameBufferQueue`.

The `isrDma()` Interrupt services routine is trigger on completion of the transfer of data to the PIO sm. On Trigger, the next_buf is submitted to the DMA, an stored as the current `frameBuffer_t`. The now old `frameBuffer_t` is then pushed to the `g_unusedFrameBufferQueue`, which is then refilled by the `gen_processingTaskFunction()` again.
Then the `isrDma()` pulls one `frameBuffer_t` from the `g_frameBufferQueue`.

This is not done on every cycle but on every `FRAME_REPEAT`nth cycle (can be configured in the [dacConfig.h](./firmware/src/dac/dacConfig.h)). There for every `frameBuffer_t` is repeated `FRAME_REPEAT`-times. This is done, to have a less flickery-image on the output-oscilloscope.

In Case of an empty `g_frameBufferQueue` the current frame is repeated (after the `FRAME_REPEAT`) until one new frame becomes available. 
