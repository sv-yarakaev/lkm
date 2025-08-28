# Defining variables
MODULE_NAME = module_rw
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
SRC = module_rw.c
obj-m += $(MODULE_NAME).o

# Default target
all:
	make -C $(KERNEL_DIR) M=$(CURDIR) modules

# Cleaning up
clean:
	make -C $(KERNEL_DIR) M=$(CURDIR) clean
	rm -f *.o *.ko *.mod* .*.cmd
	rm -rf Module.symvers modules.order

# Loading kernel module
load:
	sudo insmod $(MODULE_NAME).ko

# Unloading kernel module
unload:
	sudo rmmod $(MODULE_NAME)

# Formatting code
format:
	clang-format -i $(SRC)

# Checking code
check:
	cppcheck --enable=all --suppress=missingIncludeSystem $(SRC)

.PHONY: all clean load unload format check