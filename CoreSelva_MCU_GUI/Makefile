#####################################
# Project
#####################################

TARGET = firmware


#####################################
# Toolchain
#####################################

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size


#####################################
# MCU
#####################################

CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT = -mfloat-abi=hard

MCU = $(CPU) -mthumb $(FPU) $(FLOAT)


#####################################
# Source files
#####################################

SRC = \
main.c \
drivers/gpio.c \
drivers/systick.c \
drivers/buttons.c \
drivers/spi.c \
kernel/kernel.c \
display/st7735.c \
startup_stm32wb55.s


#####################################
# Object files
#####################################

OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:.s=.o)


#####################################
# Include folders
#####################################

INC = \
-Idrivers \
-Ikernel \
-Idisplay


#####################################
# Compiler flags
#####################################

CFLAGS = $(MCU)
CFLAGS += -Wall
CFLAGS += -O0
CFLAGS += -g
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += $(INC)


#####################################
# Linker
#####################################

LDSCRIPT = STM32WB55CGUX_FLASH.ld


LDFLAGS = $(MCU)

# use our startup file
LDFLAGS += -nostartfiles

# no operating system
LDFLAGS += -specs=nosys.specs

# memory layout
LDFLAGS += -T$(LDSCRIPT)

# remove unused code
LDFLAGS += -Wl,--gc-sections

# generate map
LDFLAGS += -Wl,-Map=$(TARGET).map


#####################################
# Build rules
#####################################

all: $(TARGET).elf


$(TARGET).elf: $(OBJ)

	$(CC) $(OBJ) $(LDFLAGS) -o $@

	$(OBJCOPY) -O binary $@ $(TARGET).bin

	$(SIZE) $@



%.o: %.c

	$(CC) $(CFLAGS) -c $< -o $@



%.o: %.s

	$(CC) $(CFLAGS) -c $< -o $@



#####################################
# Clean
#####################################

clean:

	del /S *.o *.elf *.bin *.map