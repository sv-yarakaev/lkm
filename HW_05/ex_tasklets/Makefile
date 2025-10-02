MODULE := $(notdir $(CURDIR))

# ex_bin_search/Makefile
obj-m := ex_tasklets.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

