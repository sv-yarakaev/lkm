MODULE := $(notdir $(CURDIR))

# ex_list/Makefile
obj-m += ex_list.o
ex_list-objs := src/ex_list.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

