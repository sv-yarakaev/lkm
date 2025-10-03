MODULE := $(notdir $(CURDIR))

obj-m += src/ex_timer.o
ex_bitmap-objs := src/ex_timer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
