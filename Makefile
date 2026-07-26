PROJECT := fm_drum_tracker
BUILD_DIR := build
SRC_DIR := src
INC_DIR := include
ROM := $(BUILD_DIR)/$(PROJECT).gb
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

ifndef GBDK_HOME
$(error GBDK_HOME is not set. Set it to your GBDK-2020 directory)
endif

LCC := $(GBDK_HOME)/bin/lcc
CFLAGS := -I$(INC_DIR)
LDFLAGS := -Wm-yt0x1B -Wm-yo4 -Wm-ya4 -Wm-ynFMDRUMTRACKER

.PHONY: all clean

all: $(ROM)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(ROM): $(OBJS)
	$(LCC) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(BUILD_DIR)
