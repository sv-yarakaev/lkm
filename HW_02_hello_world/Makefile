PWD := $(shell pwd)
KERNEL_VERSION ?= $(shell uname -r)
KERNEL_DIR ?= /lib/modules/$(KERNEL_VERSION)/build

SRC_DIR := $(PWD)/src
BUILD_DIR := $(PWD)/build
BUILD_FILES += $(SRC_DIR)/*.ko

CLANG_FORMAT_VERS ?= 19
CLANG_FORMAT := clang-format-$(CLANG_FORMAT_VERS)
CLANG_FORMAT_FLAGS += -i
FORMAT_FILES := $(SRC_DIR)/*.c

$(shell mkdir -p ${BUILD_DIR})

kbuild:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/src modules
	mv $(BUILD_FILES) $(BUILD_DIR)/.

format:
	$(CLANG_FORMAT) $(CLANG_FORMAT_FLAGS) $(FORMAT_FILES)

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

.PHONY: kbuild clean format
