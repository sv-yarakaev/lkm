# Defining variables
CC = gcc
CFLAGS = -Wall -g
SRC = main.c
OBJ = main.o
TARGET = app_us

# Default target
all: $(TARGET)

# Building user-space application
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

# Cleaning up
clean:
	rm -f $(OBJ) $(TARGET)

# Formatting code
format:
	clang-format -i $(SRC)

# Checking code
check:
	cppcheck --enable=all --suppress=missingIncludeSystem $(SRC)

.PHONY: all clean format check