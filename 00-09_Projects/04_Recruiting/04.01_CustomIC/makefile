# AVR Makefile

PROJECT = attiny_test
CPU = attiny85

# Structure
BUILD_DIR = build
SOURCE_DIR = src
OBJECT_DIR = $(BUILD_DIR)/obj

CC = avr-gcc
CC_FLAGS = -Wall -Os -mmcu=$(CPU)
LD_FLAGS = -mmcu=$(CPU)

C_SRC = $(wildcard $(SOURCE_DIR)/*.c)
ASM_SRC = $(wildcard $(SOURCE_DIR)/*.s)

OBJECTS = $(addprefix $(OBJECT_DIR)/,$(notdir $(C_SRC:.c=.o)))
OBJECTS += $(addprefix $(OBJECT_DIR)/,$(notdir $(ASM_SRC:.s=.o)))

all: $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT).hex

flash: $(BUILD_DIR)/$(PROJECT).hex
	minipro -p 'ATTINY85@DIP8' -w $(BUILD_DIR)/$(PROJECT).hex

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	avr-objcopy -j .text -j .data -O binary $< $@

$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	avr-objcopy -j .text -j .data -O ihex $< $@

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(LD_FLAGS) -o $@  $^

# Source Compilation
$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@
