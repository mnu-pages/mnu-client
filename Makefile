CC ?= clang
CFLAGS += -Wall -Wextra -O2 -std=c11 -Iinclude -D_POSIX_C_SOURCE=200809L
LDFLAGS += -lcurl

# Support pkg-config for easier dependency management on Linux/macOS/MSYS2
ifneq ($(shell which pkg-config),)
    CFLAGS += $(shell pkg-config --cflags libcurl 2>/dev/null)
    # LDFLAGS is handled by adding the libs
    LIBS += $(shell pkg-config --libs libcurl 2>/dev/null)
else
    LIBS += -lcurl
endif

SRC_DIR = source
OBJ_DIR = build/obj
BIN_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/mnu

all: $(TARGET)

asan: CFLAGS += -fsanitize=address -g -O1 -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address
asan: clean $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean
