###########################################################
# Toolchain
###########################################################
CC = $(MSPGCC_BIN_DIR)/msp430-elf-gcc
DEBUG = LD_LIBRARY_PATH=$(MSP_DEBUG_DRIVERS_DIR) $(MSP_DEBUG_BIN_DIR)/mspdebug
GDB = $(MSPGCC_BIN_DIR)/msp430-elf-gdb
SIZE = $(MSPGCC_BIN_DIR)/msp430-elf-size
READELF = $(MSPGCC_BIN_DIR)/msp430-elf-readelf
RM = rm

###########################################################
# Directories
###########################################################
MSPGCC_ROOT_DIR = /home/artfulbytes/ti/ccs910/ccs/tools/compiler/msp430-gcc-8.2.0.52_linux64
MSPGCC_BIN_DIR = $(MSPGCC_ROOT_DIR)/bin
MSP_DEBUG_BIN_DIR = /home/artfulbytes/ti/ccs910/ccs/ccs_base/DebugServer/bin
MSP_DEBUG_DRIVERS_DIR = /home/artfulbytes/ti/ccs910/ccs/ccs_base/DebugServer/drivers
INCLUDE_GCC_DIR = /home/artfulbytes/ti/ccs910/ccs/ccs_base/msp430/include_gcc
INCLUDE_DIRS = $(INCLUDE_GCC_DIR) ./drivers ./external/printf ./
LIB_DIRS = $(INCLUDE_GCC_DIR)
ROOT = .
SRC_DIR = $(ROOT)
OBJ_DIR = $(ROOT)/obj
BIN_DIR = $(ROOT)/bin

###########################################################
# Files
###########################################################
TARGET = $(BIN_DIR)/nsumo.out
SOURCES = main.c \
          drivers/hw.c \
          drivers/gpio.c \
          drivers/pwm.c \
          drivers/motor.c \
          drivers/adc.c \
          drivers/sleep.c \
          drivers/ir_remote.c \
          drivers/uart.c \
          drivers/led.c \
          drivers/i2c.c \
          drivers/vl53l0x.c \
          drivers/qre1113.c \
          drive.c \
          state_machine.c \
          state_machine_ir.c \
          drivers/time.c \
          opponent_detection.c \
          line_detection.c \
          test.c \
          trace.c \
          external/printf/printf.c \

OBJECT_NAMES = $(SOURCES:.c=.o)
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(OBJECT_NAMES))

###########################################################
# Flags
###########################################################
MCU = msp430g2553
CFLAGS = -mmcu=$(MCU) $(addprefix -I,$(INCLUDE_DIRS)) -DBUILD_MCU -DPRINTF_INCLUDE_CONFIG_H -Og -mmcu=msp430g2553
LDFLAGS = -mmcu=$(MCU) $(addprefix -L,$(LIB_DIRS)) -Wl,--gc-sections

###########################################################
# Build
###########################################################
$(TARGET): $(OBJECTS)
	@echo "Linking $^"
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.c
	@echo "Compiling $^"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

###########################################################
# Phony targets
###########################################################
.PHONY: all clean flash

all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJECTS)

flash: $(TARGET)
	$(DEBUG) tilib "prog $(TARGET)"

mspdeb: $(TARGET)
	$(DEBUG) tilib gdb

mspgdb:
	$(GDB)

size:
	$(SIZE) $(TARGET)

readelf:
	$(READELF) -a $(TARGET)
