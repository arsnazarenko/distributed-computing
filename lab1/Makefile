CC = clang
INCLUDE = ./include
SRC = ./src
BUILD_DIR = ./build
DEFAULT_BUILD_DIR = $(BUILD_DIR)/default
SANITIZED_BUILD_DIR = $(BUILD_DIR)/sanitized

CFLAGS += -std=c99 -Wall -Wextra -pedantic -g -I $(INCLUDE)
SANITIZED_CFLAGS = $(CFLAGS) -fsanitize=address,undefined,leak

SRCS = $(shell find ./src -type f -name *.c)
HEADS = $(shell find ./include -type f -name *.h)
DEFAULT_OBJS = $(patsubst $(SRC)/%.c, $(DEFAULT_BUILD_DIR)/%.o, $(SRCS))
SANITIZED_OBJS = $(patsubst $(SRC)/%.c, $(SANITIZED_BUILD_DIR)/%.o, $(SRCS))


all: default sanitized

default: default_dirs $(DEFAULT_BUILD_DIR)/main

sanitized: sanitized_dirs $(SANITIZED_BUILD_DIR)/main

clang-tidy:
	clang-tidy $(SRC)/*.c $(INCLUDE)/*.h -checks=-*,clang-analyzer-* -- $(CFLAGS)

default_dirs:
	mkdir -p $(DEFAULT_BUILD_DIR)

sanitized_dirs:
	mkdir -p $(SANITIZED_BUILD_DIR)

$(SANITIZED_BUILD_DIR)/%.o: $(SRC)/%.c
	$(CC) -c $(SANITIZED_CFLAGS) $< -o $@

$(DEFAULT_BUILD_DIR)/%.o: $(SRC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(SANITIZED_BUILD_DIR)/main: $(SANITIZED_OBJS)
	$(CC) $(SANITIZED_CFLAGS) $^ -o $@

$(DEFAULT_BUILD_DIR)/main: $(DEFAULT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR) pipes.log events.log

PHONY: clean default_dirs sanitized_dirs clang-tidy
