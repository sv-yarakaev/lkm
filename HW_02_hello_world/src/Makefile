KDIR := /home/stv/kernel/linux-6.1.130


.PHONY: format all
.SILENT: clean


CLANG_FORMAT_VERS ?= 15
CLANG_FORMAT := clang-format-$(CLANG_FORMAT_VERS)
CLANG_FORMAT_FLAGS += -i
FORMAT_FILES := *.c

CURRENT = $(shell uname -r)
PWD = $(shell pwd)

TARGET = ehlo
obj-m := $(TARGET).o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

format:
	$(CLANG_FORMAT) $(CLANG_FORMAT_FLAGS) $(FORMAT_FILES)

clean:
	rm -f *.ko *.cmd .*.cmd *.mod *.mod.* *.o *.order *.symvers
