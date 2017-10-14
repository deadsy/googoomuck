TOP = ../../../..
include $(TOP)/mk/common.mk

OUTPUT = ggm

# hal sources
HAL_DIR = $(TOP)/board/st/stm32f4/hal/src
SRC += $(HAL_DIR)/stm32f4xx_hal.c \
       $(HAL_DIR)/stm32f4xx_hal_rcc.c \
       $(HAL_DIR)/stm32f4xx_hal_cortex.c \
       $(HAL_DIR)/stm32f4xx_hal_gpio.c \
       $(HAL_DIR)/stm32f4xx_hal_tim.c \
       $(HAL_DIR)/stm32f4xx_hal_tim_ex.c \

# board sources
BOARD_DIR = $(TOP)/board/st/stm32f4/mb997
SRC += $(BOARD_DIR)/main.c \
       $(BOARD_DIR)/system_stm32f4xx.c \
       $(BOARD_DIR)/stm32f4xx_it.c \
       $(BOARD_DIR)/syscalls.c \
       $(BOARD_DIR)/gpio.c \

# rtt logging
RTT_DIR = $(TOP)/common/rtt
SRC += $(RTT_DIR)/SEGGER_RTT.c \
       $(RTT_DIR)/SEGGER_RTT_printf.c \

# common
COMMON_DIR = $(TOP)/common
SRC += $(COMMON_DIR)/logging.c \


OBJ = $(patsubst %.c, %.o, $(SRC))
OBJ += $(BOARD_DIR)/start.o

# include files

# include paths
INCLUDE += -I$(TOP)/board/st/stm32f4/hal/inc
INCLUDE += -I$(TOP)/board/st/stm32f4/cmsis
INCLUDE += -I$(BOARD_DIR)
INCLUDE += -I$(RTT_DIR)
INCLUDE += -I$(COMMON_DIR)

# defines
DEFINE = -DSTM32F407xx

# linker flags
LDSCRIPT = stm32f407vg_flash.ld
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
