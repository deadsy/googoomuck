TOP = .
include $(TOP)/mk/common.mk

TARGET?=mb997

TARGET_DIR = $(TOP)/target/$(TARGET)
BIN_FILE = $(TARGET_DIR)/ggm.bin

.PHONY: all program clean

all:
	make -C $(TARGET_DIR) $@

program: 
	st-flash write $(BIN_FILE) 0x08000000

clean:
	make -C $(TARGET_DIR) $@
