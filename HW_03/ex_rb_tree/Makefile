MODULE := $(notdir $(CURDIR))


ccflags-y += -I$(src)/../utils


obj-m += ex_rb_tree.o
ex_rb_tree-objs := src/ex_rb_tree.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	sudo rmmod ex_rb_tree  || echo "Already remove"
	sudo insmod ex_rb_tree.ko

