PWD := $(shell pwd)
KERNEL_VERSION ?= $(shell uname -r)
KERNEL_DIR ?= /lib/modules/$(KERNEL_VERSION)/build
SRC_DIR := $(PWD)/src
BUILD_DIR := $(PWD)/build
#BUILD_FILES += $(SRC_DIR)/*.ko

MODULE_NAME:=ehlo
MODULE_NAME_WITH_VERSION := $(MODULE_NAME)_$(KERNEL_VERSION)
BUILD_FILES += $(SRC_DIR)/$(MODULE_NAME).ko


CLANG_FORMAT_VERS ?= 20
CLANG_FORMAT := clang-format-$(CLANG_FORMAT_VERS)
CLANG_FORMAT_FLAGS += -i
FORMAT_FILES := $(SRC_DIR)/*.c

$(shell mkdir -p ${BUILD_DIR})

kbuild:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/src modules
	#mv $(BUILD_FILES) $(BUILD_DIR)/.
	mv $(SRC_DIR)/$(MODULE_NAME).ko $(BUILD_DIR)/$(MODULE_NAME_WITH_VERSION).ko

format:
	$(CLANG_FORMAT) $(CLANG_FORMAT_FLAGS) $(FORMAT_FILES)

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f $(SRC_DIR)/*.cmd $(SRC_DIR)/*.o

check:
	sudo python3 check.py
.PHONY: kbuild clean format
