TOP = ../..
include $(TOP)/mk/common.mk

OUTPUT = ggm

# hal sources
HAL_DIR = $(TOP)/soc/st/stm32f4/hal/src
SRC += $(HAL_DIR)/stm32f4xx_hal.c \
	$(HAL_DIR)/stm32f4xx_hal_rcc.c \
	$(HAL_DIR)/stm32f4xx_hal_rcc_ex.c \
	$(HAL_DIR)/stm32f4xx_hal_cortex.c \
	$(HAL_DIR)/stm32f4xx_hal_gpio.c \
	$(TOP)/soc/st/stm32f4/system_stm32f4xx.c \

# soc library sources
LIB_DIR = $(TOP)/soc/st/stm32f4/lib
SRC += $(LIB_DIR)/gpio.c \
	$(LIB_DIR)/delay.c \
	$(LIB_DIR)/i2c.c \
	$(LIB_DIR)/i2s.c \
	$(LIB_DIR)/dma.c \
	$(LIB_DIR)/adc.c \
	$(LIB_DIR)/usart.c \
	$(LIB_DIR)/rng.c \
	$(LIB_DIR)/spi.c \

# target sources
TARGET_DIR = $(TOP)/target/axoloti
SRC += $(TARGET_DIR)/main.c \
	$(TARGET_DIR)/audio.c \
	$(TARGET_DIR)/display.c \

# common
COMMON_DIR = $(TOP)/common
SRC += $(COMMON_DIR)/logging.c \
	$(COMMON_DIR)/debounce.c \
	$(COMMON_DIR)/syscalls.c \
	$(COMMON_DIR)/rand.c \
	$(COMMON_DIR)/rtt/SEGGER_RTT.c \
	$(COMMON_DIR)/rtt/SEGGER_RTT_printf.c \

# drivers
DRIVERS_DIR = $(TOP)/drivers
SRC += $(DRIVERS_DIR)/cs43l22.c \
	$(DRIVERS_DIR)/ili9341.c \

# googoomuck
GGM_DIR = $(TOP)/ggm
SRC += $(GGM_DIR)/sin.c \
	$(GGM_DIR)/midi.c \
	$(GGM_DIR)/seq.c \
	$(GGM_DIR)/ggm.c \
	$(GGM_DIR)/event.c \
	$(GGM_DIR)/adsr.c \
	$(GGM_DIR)/pan.c \
	$(GGM_DIR)/ks.c \
	$(GGM_DIR)/lpf.c \
	$(GGM_DIR)/noise.c \
	$(GGM_DIR)/block.c \
	$(GGM_DIR)/pow.c \
	$(GGM_DIR)/bmark.c \
	$(GGM_DIR)/patch0.c \
	$(GGM_DIR)/patch1.c \
	$(GGM_DIR)/patch2.c \
	$(GGM_DIR)/patch3.c \
	$(GGM_DIR)/patch4.c \
	$(GGM_DIR)/patch5.c \
	$(GGM_DIR)/patch6.c \

# ui
UI_DIR = $(TOP)/ui
SRC += $(UI_DIR)/fonts.c \
	$(UI_DIR)/term.c \
	$(UI_DIR)/graphics.c \
	$(UI_DIR)/profont29.c \

OBJ = $(patsubst %.c, %.o, $(SRC))
OBJ += $(TARGET_DIR)/start.o
OBJ += $(GGM_DIR)/p4wave.o

# include files

# include paths
INCLUDE += -I$(TOP)/soc/st/stm32f4/lib
INCLUDE += -I$(TOP)/soc/st/stm32f4/hal/inc
INCLUDE += -I$(TOP)/soc/st/stm32f4/cmsis
INCLUDE += -I$(DRIVERS_DIR)
INCLUDE += -I$(TARGET_DIR)
INCLUDE += -I$(COMMON_DIR)
INCLUDE += -I$(COMMON_DIR)/rtt
INCLUDE += -I$(GGM_DIR)
INCLUDE += -I$(UI_DIR)

# defines
DEFINE = -DSTM32F427xx
DEFINE += -DSTDIO_RTT

# linker flags
LDSCRIPT = stm32f427vg_flash.ld
X_LDFLAGS = -T$(LDSCRIPT) -Wl,-Map,$(OUTPUT).map -Wl,--gc-sections

# unused parameters in the HAL code
X_CFLAGS += -Wno-unused-parameter
X_CFLAGS += -Wno-strict-prototypes

.S.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@
.c.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(OBJ)
	$(X_GCC) $(X_CFLAGS) $(X_LDFLAGS) $(OBJ) -lm -o $(OUTPUT)
	$(X_OBJCOPY) -O binary $(OUTPUT) $(OUTPUT).bin

clean:
	-rm $(OBJ)	
	-rm $(OUTPUT)
	-rm $(OUTPUT).map	
	-rm $(OUTPUT).bin	
