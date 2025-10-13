# Внешний Makefile (в корне проекта)

.PHONY: all clean format

all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

format:
	@echo "Formatting all C source and header files..."
	@find src -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +
	@echo "Formatting complete."
