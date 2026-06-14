CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Werror -O2 -pthread
INCLUDES := -Iinclude

BUILD_DIR := build
TEST_TARGET := $(BUILD_DIR)/atomic_max_tests
BENCHMARK_TARGET := $(BUILD_DIR)/atomic_max_benchmark

.PHONY: all test benchmark run-benchmark clean

all: $(TEST_TARGET) $(BENCHMARK_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_TARGET): tests/AtomicMaxTests.cpp include/AtomicMax.h include/AtomicMaxWithLock.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) tests/AtomicMaxTests.cpp -o $(TEST_TARGET)

$(BENCHMARK_TARGET): benchmark/AtomicMaxBenchmark.cpp include/AtomicMax.h include/AtomicMaxWithLock.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) benchmark/AtomicMaxBenchmark.cpp -o $(BENCHMARK_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

benchmark: $(BENCHMARK_TARGET)

run-benchmark: $(BENCHMARK_TARGET)
	./$(BENCHMARK_TARGET)

clean:
	rm -rf $(BUILD_DIR)
