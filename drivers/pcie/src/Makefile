
obj-m := pciDriver.o
pciDriver-objs := base.o int.o umem.o kmem.o sysfs.o ioctl.o

# needs to be set by the parent Makfile
VERSION ?= $(shell ./version.sh)
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
INSTALLDIR ?= /lib/modules/$(shell uname -r)/extra
INSTALLHDRDIR ?= /usr/include/pciDriver/driver
INSTALLDKMSDIR ?= /usr/src/pciDriver-$(VERSION)
PWD := $(shell pwd)

DKMS_FILES := $(wildcard *.h) $(wildcard *c) dkms.conf Makefile version.sh

default: dkms.conf
	$(MAKE) -C $(KERNELDIR) M=$(PWD) VERSION=$(VERSION) modules

dkms.conf: dkms.conf.in
	sed 's/@VERSION@/$(VERSION)/g' $< > $@

install-dkms: dkms.conf
	@mkdir -p $(INSTALLDKMSDIR)
	@echo "INSTALL pciDriver files for DKMS usage"
	@cp --preserve=mode $(DKMS_FILES) $(INSTALLDKMSDIR)
	@/usr/sbin/dkms add -m pciDriver -v $(VERSION)
	@/usr/sbin/dkms build -m pciDriver -v $(VERSION)
	@/usr/sbin/dkms install -m pciDriver -v $(VERSION)

uninstall-dkms:
	@/usr/sbin/dkms remove -m pciDriver -v $(VERSION) --all
	@echo "UNINSTALL pciDriver files for DKMS usage"
	@$(foreach dkms_file, $(DKMS_FILES), rm -f $(INSTALLDKMSDIR)/$(dkms_file))
	@rmdir $(INSTALLDKMSDIR)

install:
	@mkdir -p $(INSTALLDIR)
	@echo "INSTALL $(INSTALLDIR)/pciDriver.ko"
	@install -m 755 pciDriver.ko $(INSTALLDIR)
	@echo "INSTALL $(INSTALLHDRDIR)/pciDriver.h"
	@mkdir -p $(INSTALLHDRDIR)
	@install -m 644 pciDriver.h $(INSTALLHDRDIR)

uninstall:
	@echo "UNINSTALL $(INSTALLDIR)/pciDriver.ko"
	@rm -f $(INSTALLDIR)/pciDriver.ko
	@echo "UNINSTALL $(INSTALLHDRDIR)/pciDriver.h"
	@rm -rf /usr/include/pciDriver/driver

clean:
	rm -f *.o *.ko *.mod.c .*.cmd Module.symvers modules.order
	rm -rf .tmp_versions
