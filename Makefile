CC = gcc

APP_CFLAGS    = -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter -g
VENDOR_CFLAGS = -Wall -Wextra -Wno-unused-parameter -g

INCLUDES = -Iinc -Ivendor/inc -I/usr/include -IEGL -IGL -Iwayland-egl
LIBS     = -lwayland-client -lEGL -lGL -lwayland-egl

TARGET = wl-paper

BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
OUT_DIR   = $(BUILD_DIR)/out

SRC_DIRS = src vendor
SOURCES  = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJECTS  = $(SOURCES:%.c=$(OBJ_DIR)/%.o)

all: dirs $(OUT_DIR)/$(TARGET)

dirs:
	mkdir -p $(OBJ_DIR)/src $(OBJ_DIR)/vendor $(OUT_DIR)

$(OUT_DIR)/$(TARGET): $(OBJECTS)
	$(CC) $(APP_CFLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR)/src/%.o: src/%.c | dirs
	$(CC) $(APP_CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/vendor/%.o: vendor/%.c | dirs
	$(CC) $(VENDOR_CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean dirs
