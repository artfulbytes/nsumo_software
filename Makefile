###########################################################
# Toolchain
###########################################################
CC = $(MSPGCC_BIN_DIR)/msp430-elf-gcc
DEBUG = LD_LIBRARY_PATH=$(MSP_DEBUG_DRIVERS_DIR) $(MSP_DEBUG_BIN_DIR)/mspdebug
GDB = $(MSPGCC_BIN_DIR)/msp430-elf-gdb
SIZE = $(MSPGCC_BIN_DIR)/msp430-elf-size
READELF = $(MSPGCC_BIN_DIR)/msp430-elf-readelf
OBJDUMP = $(MSPGCC_BIN_DIR)/msp430-elf-objdump
RM = rm

###########################################################
# Directories
###########################################################
TI_CCS_DIR = /home/ab/dev/tools/ccs1110/ccs
MSPGCC_ROOT_DIR = /home/ab/dev/tools/msp430-gcc
MSPGCC_BIN_DIR = $(MSPGCC_ROOT_DIR)/bin
MSP_DEBUG_BIN_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/bin
MSP_DEBUG_DRIVERS_DIR = $(TI_CCS_DIR)/ccs_base/DebugServer/drivers
INCLUDE_GCC_DIR = $(TI_CCS_DIR)/ccs_base/msp430/include_gcc
INCLUDE_DIRS = $(INCLUDE_GCC_DIR) ./drivers ./state_machine ./external/printf ./
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
          drivers/ir_remote.c \
          drivers/uart.c \
          drivers/led.c \
          drivers/i2c.c \
          drivers/vl53l0x.c \
          drivers/qre1113.c \
          drive.c \
          state_machine/state_machine.c \
          state_machine/state_test.c \
          state_machine/state_search.c \
          state_machine/state_attack.c \
          state_machine/state_retreat.c \
          state_machine_ir.c \
          drivers/millis.c \
          enemy_detection.c \
          line_detection.c \
          detection_history.c \
          test.c \
          trace.c \
          external/printf/printf.c \
          timer.c \

FILES_TO_FORMAT = *.c \
                  *.h \
                  drivers/*.h \
                  drivers/*.c \
                  state_machine/*.c \
                  state_machine/*.h \

OBJECT_NAMES = $(SOURCES:.c=.o)
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(OBJECT_NAMES))
# Allow passing macro defines as parameter to make (e.g. "make DEFINES="BUILD_TEST")
LOCAL_DEFINES = $(addprefix -D,$(DEFINES))
# Select a test to build (e.g. "make TEST="test_dimming_led")
TEST_DEFINE = $(addprefix -DTEST=,$(TEST))

###########################################################
# Flags
###########################################################
MCU = msp430g2553
WFLAGS = -Wall -Wextra -Werror -Wshadow
CFLAGS += -mmcu=$(MCU) $(addprefix -I,$(INCLUDE_DIRS)) -DBUILD_MCU $(LOCAL_DEFINES) $(TEST_DEFINE) -DPRINTF_INCLUDE_CONFIG_H -Og -mmcu=msp430g2553 $(WFLAGS)
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
.PHONY: all format clean flash mspdeb mspgdb size readelf disassemble

all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJECTS)

format:
	clang-format -i --style=file $(FILES_TO_FORMAT)

flash: $(TARGET)
	$(DEBUG) tilib "prog $(TARGET)"

mspdeb: $(TARGET)
	$(DEBUG) tilib gdb

mspgdb:
	$(GDB)

size: $(TARGET)
	$(SIZE) $(TARGET)

# List symbol table sorted by size
symbols: $(TARGET)
	$(READELF) -s $(TARGET) | sort -n -k3

disassemble: $(TARGET)
	$(OBJDUMP) --disassemble-all $(TARGET)
