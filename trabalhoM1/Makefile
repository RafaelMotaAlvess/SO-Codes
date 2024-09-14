# Definições de variáveis
CC = gcc
CFLAGS = -Wall -pthread
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

SRC = main.c task_queue.c socket_handler.c utils.c
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
TARGET = $(BUILD_DIR)/server

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
