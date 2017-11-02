TUSB = tinyusb-master
WORK_DIR = $(TUSB)
ZIPFILE = $(TUSB).zip

PATCHFILES = $(sort $(wildcard patches/*.patch))

PATCH_CMD = \
        for f in $(PATCHFILES); do\
            echo $$f; \
            patch -s -d $(WORK_DIR) -p1 -b -z .original < $$f; \
        done

extract:
	unzip -q $(ZIPFILE)
	$(PATCH_CMD)
	cd files; tar -c -f - * | (cd ../$(WORK_DIR) ; tar xfp -)

host: extract
	make -C $(WORK_DIR) -f Makefile.host libusbhost.a
	make -C $(WORK_DIR) -f Makefile.host demo

device: extract
	make -C $(WORK_DIR) -f Makefile.device
	make -C $(WORK_DIR) -f Makefile.device demo

clean:
	-rm -rf $(WORK_DIR)
