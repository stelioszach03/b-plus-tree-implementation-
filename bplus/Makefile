CC = gcc
CFLAGS = -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ -Wall -Wextra -g -O2
LIBS = -lbf
BUILD_DIR = ./build
SRC_DIR = ./src
EXAMPLES_DIR = ./examples

SRCS = $(SRC_DIR)/record.c $(SRC_DIR)/bp_file.c $(SRC_DIR)/bp_datanode.c $(SRC_DIR)/bp_indexnode.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

bplus: $(OBJS)
	@echo " Compile bplus_main ..."
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/bp_main.c $(OBJS) $(LIBS) -o $(BUILD_DIR)/bplus_main

test: $(OBJS)
	@echo " Compile bp_test ..."
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/bp_test.c $(OBJS) $(LIBS) -o $(BUILD_DIR)/bp_test

.PHONY: clean
clean:
	@echo " Cleaning..."
	@rm -rf $(BUILD_DIR)/*
	@rm -f *.db

