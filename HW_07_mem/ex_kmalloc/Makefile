MODULE := $(notdir $(CURDIR))

ccflags-y += -I$(src)/../utils
obj-m += $(MODULE).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
