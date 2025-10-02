# Defining variables
APP_DIR = app_us
LKM_DIR = lkm

# Default target
all: app_us lkm

# Building user-space application
app_us:
	$(MAKE) -C $(APP_DIR)

# Building kernel module
lkm:
	$(MAKE) -C $(LKM_DIR)

# Cleaning up
clean:
	$(MAKE) -C $(APP_DIR) clean
	$(MAKE) -C $(LKM_DIR) clean

# Loading kernel module
load:
	$(MAKE) -C $(LKM_DIR) load

# Unloading kernel module
unload:
	$(MAKE) -C $(LKM_DIR) unload

# Formatting code
format:
	$(MAKE) -C $(APP_DIR) format
	$(MAKE) -C $(LKM_DIR) format

# Checking code
check:
	$(MAKE) -C $(APP_DIR) check
	$(MAKE) -C $(LKM_DIR) check

.PHONY: all app_us lkm clean load unload format check