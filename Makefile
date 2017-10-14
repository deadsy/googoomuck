TOP = .
include $(TOP)/mk/common.mk

TARGET_DIR = $(TOP)/target/mb997
BIN_FILE = $(TARGET_DIR)/ggm.bin

.PHONY: all program clean

all:
	make -C $(TARGET_DIR) $@

program: 
	st-flash write $(BIN_FILE) 0x08000000

clean:
	make -C $(TARGET_DIR) $@
