# Nsumo code
This repository contains code for Nsumo, a mini-class Sumobot (500g 10x10cm). Nsumo has
four DC-motors, three range sensors, four line sensors, and a [custom-designed PCB](https://github.com/artfulbytes/nsumo_hardware.git)
including the microcontroller MSP430G2553. The code is bare-metal C and follows a [super-loop
architecture](https://en.wikibooks.org/wiki/Embedded_Systems/Super_Loop_Architecture). It includes
a state machine implementation and many low-level drivers for GPIO, I2C, ADC, UART, VL53L0X, and more.

<img src="/images/nsumo.jpg">

## Files
| Folder | Description |
|-----|----|
| / | mostly higher-level code that abstracts the driver code in drivers/ |
| drivers/ | Low-level driver code
| state_machine/ | State machine code
| external/ | 3rd-party code

| Filename | Description |
|-----|----|
| state_machine | Entry point for state machine |
| state_attack | Translates enemy position to appropriate attack move |
| state_retreat | Translates line detection to appropriate retreat move |
| state_search | Shifts search strategy based on time elapsed |
| state_common | Shared definitions |
| led | Function to initialize and configure GPIO for LED |
| gpio | GPIO handling for initialization and interrupt configuration |
| hw | Microcontroller initialization (timer, watchdog, etc.) |
| millis | Get time passed using watchdog timer |
| timer | Timer creation using the millis function |
| ir_remote | Decodes NEC protocol (using a timer 1) for remote control |
| drive | Abstracts motor control into discrete drive speeds and directions |
| motor | Motor control layer that abstracts the PWM functions |
| pwm | Hardware PWM using timer 0 |
| line_detection | Maps qre1113 detections into discrete enum values |
| qre1113 | Implements line sensor driver using ADC |
| adc | ADC driver with DMA |
| enemy_detection | Maps vl53l0x detections into discrete enum values |
| vl53l0x | Implements range sensor driver using I2C |
| i2c | I2C driver |
| detection_history | Implements a ring buffer to store previous sensor detections |
| external/printf | printf implementation suitable for embedded systems |
| trace | Trace wrapper adding prefix and log level to printf |
| uart | UART driver |

# Setup
The code is written specifically for MSP430G2553 (28 pin version) and won't run on any
other microcontroller as is. Also, all of the code won't be applicable unless
your sumobot has the same hardware as Nsumo. You are more likely to want to port
parts of the code to your own project, which should be easy enough.

Anyway, if you want to set up this project, I will explain how in the following sections.

## Clone the repo
Clone the repo and its submodules.
```
git clone --recursive https://github.com/artfulbytes/nsumo.git
```

## Install Code Composer Studio (CCS)
This project can be built and debugged with CCS, an IDE by Texas Instrument. It's
free and can be found at their [website](https://www.ti.com/tool/CCSTUDIO). Import
the project and press the hammer to build it.

## GCC compiler and Makefile
I've also created a Makefile to compile the project from the command line. For this
to work, you must install CCS (see the previous section) and download the [GCC compiler for MSP430](https://www.ti.com/tool/MSP430-GCC-OPENSOURCE).

Make sure you modify the MSPGCC and TI_CCS_DIR variables inside Makefile to match the paths
on your computer.

Then in the project root, simply run
```
make
```

## Simulation (Bots2D)
The state machine logic can be simulated inside my robotics simulator [Bots2D](https://github.com/artfulbytes/bots2d),
see that repository for more info.

<img src="/images/simulator.png">

# Clang format and Clangd
This project uses clang-format to autoformat the code. With clang-format installed, you can format the code by running
```
make format
```

To get LSP features with Clangd, you can use [bear](https://github.com/rizsotto/Bear) to generate a compilation database
```
make clean
bear -- make
