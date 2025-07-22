MODULE := $(notdir $(CURDIR))

# ex_list/Makefile
obj-m += ex_bin_search.o
ex_list-objs := src/ex_bin_search.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

