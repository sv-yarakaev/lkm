MODULE := $(notdir $(CURDIR))

# ex_list/Makefile
obj-m += ex_queue.o
ex_queue-objs := src/ex_queue.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
