#Makefile for external modules in Linux kernel 2.6.11
MODULE  = siplx.ko
LSMOD   = /sbin/lsmod
INSMOD  = sudo /sbin/insmod
RMMOD   = sudo /sbin/rmmod
RM      = @rm -rf
MODULE_NAME = siplx
$(MODULE_NAME)-objs = main.o init.o plx.o plx0206.o isr.o busmaster.o 

# First pass, kernel Makefile reads module objects
ifneq ($(KERNELRELEASE),)
obj-m	:= $(MODULE_NAME).o

#Uncomment to build debug
#EXTRA_CFLAGS += -DSIDebugLevelAll

# Second pass, the actual build.
else
KVER    ?= $(shell uname -r)
KDIR	:= /lib/modules/$(KVER)/build
PWD	:= $(shell pwd)

all:
	$(MAKE) ${EXTRA_CFLAGS} -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean


# Indents the kernel source the way linux/Documentation/CodingStyle.txt
# wants it to be.
indent:
	indent -kr -i8 $($(MODULE_NAME)-objs:.o=.c)

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

endif
