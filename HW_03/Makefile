
SUBDIRS := ex_list ex_bin_search ex_rb_tree
export KBUILD_CFLAGS += -fanalyzer
export EXTRA_CFLAGS += -Wall -Werror -Wextra -Wpointer-arith    

all:
	@for dir in $(SUBDIRS); do \
		echo "Building $$dir..."; \
		$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd)/$$dir modules || { echo "Build failed for $$dir"; exit 1; }; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		echo "Cleaning $$dir..."; \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: all clean
