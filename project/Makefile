KERNEL_VER := $(shell uname -r)
MODULE_NAME := pr_blk
CUSTOM_KERNEL_DIR := /home/stv/kernel/linux-6.1.130

ifneq ($(wildcard $(CUSTOM_KERNEL_DIR)),)
    KERNEL_DIR := $(CUSTOM_KERNEL_DIR)
KERNEL_VER := 6.1.130
else
    KERNEL_DIR := /lib/modules/$(shell uname -r)/build
endif

# Проверка существования
ifneq ($(wildcard $(KERNEL_DIR)),)
    # OK
else
    $(error Required kernel source/build directory "$(KERNEL_DIR)" not found)
endif

# Далее ваша сборка...
obj-m += src/$(MODULE_NAME).o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
