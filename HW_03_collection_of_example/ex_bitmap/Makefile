MODULE := $(notdir $(CURDIR))

ccflags-y += -I$(src)/../utils
obj-m += ex_bitmap.o
ex_bitmap-objs := src/ex_bitmap.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
