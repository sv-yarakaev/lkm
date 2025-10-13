MODULE := $(notdir $(CURDIR))

obj-m += src/ex_timer.o
ex_bitmap-objs := src/ex_timer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: clean all
	sudo insmod src/ex_timer.ko

unload:
	sudo rmmod ex_timer

format:
	clang-format -i src/ex_timer.c
