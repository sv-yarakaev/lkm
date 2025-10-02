MODULE := $(notdir $(CURDIR))

obj-m := ex_workqueue.o
#ex_tasklets-objs := ex_tasklets.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	sudo rmmod ex_bin_search || echo "Already remove"
	sudo insmod ex_bin_search.ko

