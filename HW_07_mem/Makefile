
SUBDIRS := ex_get_page  ex_kmalloc  ex_kmem_cache  ex_mempool  ex_vmalloc
export KBUILD_CFLAGS += -fanalyzer
export EXTRA_CFLAGS += -Wall -Werror -Wextra -Wpointer-arith    

FORMAT_EXT := c h 
# Найти все файлы с нужными расширениями в подкаталогах
FORMAT_FILES := $(foreach ext,$(FORMAT_EXT),$(wildcard $(addsuffix / *.$(ext),$(SUBDIRS))))

format:
	@if command -v clang-format >/dev/null 2>&1; then \
        for dir in $(SUBDIRS); do \
                        echo "Formatting files in $$dir..."; \
                        for file in $$dir/*.c $$dir/*.h; do \
                                if [ -f "$$file" ]; then \
                                        echo "  Formatting $$file"; \
                                        clang-format -i "$$file"; \
                                fi; \
                        done; \
                done; \
                echo "Formatting completed"; \
        else \
                echo "Error: clang-format not found. Please install it."; \
                exit 1; \
        fi	



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


compile_commands.json:
	bear --make

cc: compile_commands.json
	@echo "compile_commands.json generated"

.PHONY: all clean
