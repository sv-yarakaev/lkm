SUBDIRS := ex_bin_search ex_bitmap ex_list ex_queue ex_rb_tree
BUILD_DIR := build

.PHONY: all clean $(SUBDIRS)

all: $(BUILD_DIR)
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done
	@find $(SUBDIRS) -name '*.ko' -exec cp -v {} $(BUILD_DIR)/ \;

$(SUBDIRS): $(BUILD_DIR)
	@$(MAKE) -C $@
	@find $@ -name '*.ko' -exec cp -v {} $(BUILD_DIR)/ \;

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -rf $(BUILD_DIR)/*
