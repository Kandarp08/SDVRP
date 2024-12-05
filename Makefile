# Compiler and flags
CXX = g++

# Directories
SRC_DIR = src
INTRA_DIR = $(SRC_DIR)/intra_operator
INTER_DIR = $(SRC_DIR)/inter_operator
BUILD_DIR = build
DATA_DIR = data
OUTPUT_DIR = output

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp $(INTRA_DIR)/*.cpp $(INTER_DIR)/*.cpp) main.cpp
OBJS = $(SRCS:.cpp=.o)

# Executable
TARGET = $(BUILD_DIR)/test_runner

# Default rule
all: build

# Build the executable
build: $(SRCS)
	mkdir -p $(BUILD_DIR)
	$(CXX) -o $(TARGET) $(SRCS)

# Run all test cases
run-all: build
	echo "1 21" | $(TARGET)

# Run a range of test cases
run-range: build
	@echo "Enter range as '<start_index> <end_index>'"
	@read l r; echo "$$l $$r" | $(TARGET)

# Run a single test case
run-one: build
	@echo "Enter test case number (1 to 21)"
	@read i; echo "$$i $$i" | $(TARGET)

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)/*.txt
