CC = clang
INCLUDE = include
SRC = src
BUILD = build
CFLAGS += -std=c17 -Wall -Werror -Wextra -pedantic -I $(INCLUDE)
OBJS = $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(wildcard $(SRC)/*.c))

all: directories $(BUILD)/main

clang-tidy:
	clang-tidy $(src)/*.c -checks=-*,clang-analyzer-* -- $(CFLAGS)

directories:
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/main: $(OBJS)
	$(CC) $(FLAGS) $^ -o $@

clean:
	rm -rf $(BUILD)

PHONY: clean directories clang-tidy
