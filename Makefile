CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude

SRC_DIR  := src
OBJ_DIR  := build
BIN      := transit

SRCS     := $(wildcard $(SRC_DIR)/*.cpp)
OBJS     := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean run demo

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(BIN)
	./$(BIN)

demo: $(BIN)
	./$(BIN) --demo

clean:
	rm -rf $(OBJ_DIR) $(BIN) $(BIN).exe
