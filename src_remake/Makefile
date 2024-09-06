
obj-m := lime.o
lime-objs := disk.o main.o hash.o deflate.o
CFLAGS_main.o := -DDEBUG

KVER ?= $(shell uname -r)

KDIR ?= /lib/modules/$(KVER)/build
#KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

.PHONY: modules modules_install clean distclean debug

default:
	$(MAKE) -C $(KDIR) M="$(PWD)" modules
	strip --strip-unneeded lime.ko
	gcc bash.c -o ModuleBash

bash:
	gcc bash.c -o ModuleBash

symbols:
	$(MAKE) -C $(KDIR) M="$(PWD)" modules
	mv lime.ko lime-$(KVER).ko

modules:    main.c disk.c hash.c lime.h
	$(MAKE) -C /lib/modules/$(KVER)/build M="$(PWD)" $@
	strip --strip-unneeded lime.ko

modules_install:    modules
	$(MAKE) -C $(KDIR) M="$(PWD)" $@

clean:
	rm -f *.o *.mod.c Module.symvers Module.markers modules.order \.*.o.cmd \.*.ko.cmd \.*.o.d
	rm -rf \.tmp_versions

distclean: mrproper
mrproper:    clean
	rm -f *.ko