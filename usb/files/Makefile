TOP = ../..
include $(TOP)/mk/common.mk

SRC = tinyusb/osal/osal_none.c \
      tinyusb/host/hcd.c \
      tinyusb/host/hub.c \
      tinyusb/host/usbh.c \

OBJ = $(SRC:.c=.o)

# include paths
INCLUDE += -I$(TOP)/usb/tinyusb-master/tinyusb
INCLUDE += -I$(TOP)/soc/st/stm32f4/cmsis

# turn off some warnings
#X_CFLAGS += -Wno-strict-prototypes
#X_CFLAGS += -Wno-unused-parameter

.c.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@

libusb.a: $(OBJ)
	$(X_AR) rcs $@ $(OBJ)

clean:
	rm -f *.o *.a
