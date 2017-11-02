TUSB = tinyusb-master
WORK_DIR = $(TUSB)
ZIPFILE = $(TUSB).zip

PATCHFILES = $(sort $(wildcard patches/*.patch))

PATCH_CMD = \
        for f in $(PATCHFILES); do\
            echo $$f; \
            patch -s -d $(WORK_DIR) -p1 -b -z .original < $$f; \
        done

all:
	unzip -q $(ZIPFILE)
	$(PATCH_CMD)
	cd files; tar -c -f - * | (cd ../$(WORK_DIR) ; tar xfp -)
	make -C $(WORK_DIR) libusb.a

clean:
	-rm -rf $(WORK_DIR)
