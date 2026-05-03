CC ?= clang
CFLAGS += -Wall -Wextra -O2 -std=c11 -Iinclude -D_POSIX_C_SOURCE=200809L

# OS Detection
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
else
    PLATFORM := $(shell uname -s)
endif

# Base dependencies
ifneq ($(shell which pkg-config),)
    # Use pkg-config to find libcurl and its dependencies for static linking
    CFLAGS += $(shell pkg-config --cflags libcurl 2>/dev/null)
    LIBS += $(shell pkg-config --libs --static libcurl 2>/dev/null || pkg-config --libs libcurl)
else
    LIBS += -lcurl
endif

# Platform-specific static adjustments
ifeq ($(PLATFORM),Windows)
    # On Windows, we need to define CURL_STATICLIB to prevent DLL export lookups
    # and use -static to link the C runtime and other libs statically.
    CFLAGS += -DCURL_STATICLIB
    LDFLAGS += -static
endif

# Note: On Linux, full -static is often problematic with glibc.
# We prioritize static libcurl via pkg-config --static instead.

SRC_DIR = source
OBJ_DIR = build/obj
BIN_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Deterministic output path: can be overridden by CI
OUT ?= $(BIN_DIR)/mnu
TARGET = $(OUT)

all: $(TARGET)

# AddressSanitizer build for debugging
asan: CFLAGS += -fsanitize=address -g -O1 -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address
asan: clean $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $(TARGET))
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean asan
