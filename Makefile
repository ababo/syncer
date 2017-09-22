CXX_FLAGS := -std=c++11 -Iinclude -Ithird_party -O3
LD_FLAGS := -lzmq

SRC_DIR := ./test
BLD_DIR := ./build
BIN_DIR := $(BLD_DIR)/test
DOC_DIR := $(BLD_DIR)/doc

SRCS := main.cc sanity.cc
OBJS := $(SRCS:%.cc=$(BIN_DIR)/%.o)
DEPS := $(SRCS:%.cc=$(BIN_DIR)/%.d)
BIN := $(BIN_DIR)/test

-include $(DEPS)

all: test doc

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cc
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXX_FLAGS) -MMD $< -c -o $@

test: $(OBJS)
	$(CXX) $(LD_FLAGS) $(OBJS) -o $(BIN)

run-test: $(BIN)
	$(BIN)

doc:
	mkdir -p $(DOC_DIR)
	doxygen

open-doc:
	open $(DOC_DIR)/index.html

clean:
	rm -r $(BLD_DIR)
