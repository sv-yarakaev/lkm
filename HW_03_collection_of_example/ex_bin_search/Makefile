MODULE := $(notdir $(CURDIR))

# ex_bin_search/Makefile
obj-m += ex_bin_search.o
ex_bin_search-objs := src/ex_bin_search.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	sudo rmmod ex_bin_search || echo "Already remove"
	sudo insmod ex_bin_search.ko

