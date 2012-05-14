#******************************************************************************
#*
#*
#******************************************************************************

obj-m := voicesolution.o
voicesolution-objs	:= samsung_voicesolution.o

EXTRA_CFLAGS += -I./include

KDIR	:= /lib/modules/$(shell uname -r)/build

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
#	cp -a voicesolution.ko /system/data/voicesolution.ko
	
clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf *.bak
	
