CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -pthread
CPPFLAGS := -Iinclude
LDLIBS :=

BUILD_DIR := build
RESULTS_DIR := results

THREAD_POOL_SOURCE := src/ThreadPool.cpp
TEST_SOURCE := tests/ThreadPoolTests.cpp
BENCHMARK_SOURCE := benchmark/ThreadPoolBenchmark.cpp

TEST_TARGET := $(BUILD_DIR)/thread_pool_tests
BENCHMARK_TARGET := $(BUILD_DIR)/thread_pool_benchmark
BENCHMARK_CSV := $(RESULTS_DIR)/benchmark.csv
BENCHMARK_GRAPH := $(RESULTS_DIR)/thread_pool_benchmark.png

.PHONY: all test benchmark run-benchmark graph clean

all: $(TEST_TARGET) $(BENCHMARK_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(RESULTS_DIR):
	mkdir -p $(RESULTS_DIR)

$(TEST_TARGET): $(THREAD_POOL_SOURCE) $(TEST_SOURCE) include/ThreadPool.h include/ThreadPoolWithLock.h include/AsioThreadPool.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(THREAD_POOL_SOURCE) $(TEST_SOURCE) -o $(TEST_TARGET) $(LDLIBS)

$(BENCHMARK_TARGET): $(THREAD_POOL_SOURCE) $(BENCHMARK_SOURCE) include/ThreadPool.h include/ThreadPoolWithLock.h include/AsioThreadPool.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(THREAD_POOL_SOURCE) $(BENCHMARK_SOURCE) -o $(BENCHMARK_TARGET) $(LDLIBS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

benchmark: $(BENCHMARK_TARGET)

run-benchmark: $(BENCHMARK_TARGET) | $(RESULTS_DIR)
	./$(BENCHMARK_TARGET) > $(BENCHMARK_CSV)
	@echo "Результаты сохранены в $(BENCHMARK_CSV)"

graph: $(BENCHMARK_CSV)
	python3 scripts/plot_benchmark.py $(BENCHMARK_CSV) $(BENCHMARK_GRAPH)

$(BENCHMARK_CSV): $(BENCHMARK_TARGET) | $(RESULTS_DIR)
	./$(BENCHMARK_TARGET) > $(BENCHMARK_CSV)

clean:
	rm -rf $(BUILD_DIR) $(RESULTS_DIR)
