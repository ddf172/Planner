.PHONY: all build clean

BUILD_DIR := build

all: build

build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && $(MAKE)

clean:
	rm -rf $(BUILD_DIR)