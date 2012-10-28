MODULE = humble
PWD = $(shell pwd)

HEADERS = /lib/modules/$(shell uname -r)/build

obj-m := $(MODULE).o
$(MODULE)-objs := main.o clandestine.o hashtable.o chardev.o

all:
	$(MAKE) -C $(HEADERS) M=$(PWD) modules

clean:
	$(MAKE) -C $(HEADERS) M=$(PWD) clean

load:
	insmod ./$(MODULE).ko

unload:
	rmmod ./$(MODULE).ko
