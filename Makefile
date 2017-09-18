CXX_FLAGS := -std=c++11 -I. -Ithird_party

BUILD_DIR := ./build
DOC_DIR := ./doc

TESTS_BINARY := ./build/syncer_test

all: tests

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

tests: $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) test/main.cc -O3 -o $(TESTS_BINARY)

test: tests
	$(TESTS_BINARY)

doc:
	doxygen

open-doc: doc
	open $(DOC_DIR)/index.html

clean:
	rm -r $(DOC_DIR) $(BUILD_DIR)

.PHONY: all tests test doc open-doc clean
