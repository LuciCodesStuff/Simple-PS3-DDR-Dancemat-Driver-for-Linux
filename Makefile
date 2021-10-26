obj-m += hid-spsdm.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	#make -C $(KDIR)  M=$(shell pwd) clean
	rm -rf *.ko *.mod *.mod.c *.cmd *.order *.mod.o
