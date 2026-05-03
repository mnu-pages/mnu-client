CC ?= clang

CFLAGS = -Wall -Wextra -O2 -std=c11 -Iinclude
LIBS = -lcurl

SRC = source
BUILD = build
TARGET = $(BUILD)/mnu

SOURCES = $(wildcard $(SRC)/*.c)

all:
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

clean:
	rm -rf build

.PHONY: all clean
