TOP = ..
include $(TOP)/mk/common.mk


DATE = 20140628
ZIP = goom-$(DATE).zip
OUTPUT = goom

PATCHFILES = $(sort $(wildcard *.patch))

PATCH_CMD = \
				for f in $(PATCHFILES); do\
					echo $$f; \
					patch -s -d . -p1 -b -z .original < $$f; \
				done

C_SRC = $(DATE)/main.c

S_SRC = $(DATE)/startup.s \
        $(DATE)/wave.s \

OBJ = $(patsubst %.c, %.o, $(C_SRC))
OBJ += $(patsubst %.s, %.o, $(S_SRC))

# compiler flags
X_CFLAGS = -Werror -Wall -Wextra -Wstrict-prototypes
X_CFLAGS += -Os
X_CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3 -mthumb-interwork
X_CFLAGS += -fno-builtin-strcpy -fno-builtin-memcpy -fno-builtin-memset -fno-jump-tables

# linker flags
LDSCRIPT = $(DATE)/main.lnk
X_LDFLAGS = -T$(LDSCRIPT) -Wl,-Map,$(OUTPUT).map -Wl,--gc-sections

.s.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@
.c.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@

all: goom_source $(OBJ)
	$(X_GCC) $(X_CFLAGS) $(X_LDFLAGS) $(OBJ) -o $(OUTPUT)
	$(X_OBJCOPY) -O binary $(OUTPUT) $(OUTPUT).bin

goom_source: $(ZIP)
	unzip $(ZIP)
	$(PATCH_CMD)

clean:
	-rm -rf $(DATE)
	-rm $(OUTPUT)
	-rm $(OUTPUT).bin
	-rm $(OUTPUT).map
