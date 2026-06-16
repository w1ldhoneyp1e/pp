CXX := g++

TARGET := build/histogram_lab

THREAD_FLAGS := -pthread

CXXFLAGS := \
	-std=c++20 \
	-O2 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-MMD \
	-MP \
	$(THREAD_FLAGS)

CPPFLAGS := \
	-Iinclude \
	$(shell pkg-config --cflags libpng)

LDFLAGS := $(THREAD_FLAGS)

LDLIBS := $(shell pkg-config --libs libpng)

SOURCES := $(shell find src -type f -name '*.cpp' | sort)
OBJECTS := $(patsubst src/%.cpp,build/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

IMAGE ?= images/image.png
REPETITIONS ?= 5
THREADS ?= 1,2,4,8,16
RESULTS ?= results/benchmark.csv

SYNTHETIC_SIZE ?= 6000x4000
PATTERN ?= random

ARGS ?=

.PHONY: \
	all \
	run \
	benchmark \
	synthetic \
	test \
	graph \
	benchmark-graph \
	clean \
	clean-results \
	check-deps

all: check-deps $(TARGET)

check-deps:
	@pkg-config --exists libpng || \
		(echo "Ошибка: libpng не найдена. Установите: sudo apt install libpng-dev"; exit 1)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET) $(ARGS)

benchmark: all
	@mkdir -p $(dir $(RESULTS))
	./$(TARGET) \
		--image "$(IMAGE)" \
		--repetitions "$(REPETITIONS)" \
		--threads "$(THREADS)" \
		--output "$(RESULTS)"

synthetic: all
	@mkdir -p $(dir $(RESULTS))
	./$(TARGET) \
		--synthetic "$(SYNTHETIC_SIZE)" \
		--pattern "$(PATTERN)" \
		--repetitions "$(REPETITIONS)" \
		--threads "$(THREADS)" \
		--output "$(RESULTS)"

test: all
	@mkdir -p results
	./$(TARGET) \
		--synthetic 1024x1024 \
		--pattern gradient \
		--repetitions 2 \
		--threads 1,2,4 \
		--output results/test.csv

graph:
	@test -f "$(RESULTS)" || \
		(echo "Ошибка: файл $(RESULTS) не найден. Сначала выполните make benchmark или make synthetic."; exit 1)
	python3 scripts/plot_benchmark.py \
		"$(RESULTS)" \
		results

benchmark-graph: benchmark graph

clean:
	rm -rf build

clean-results:
	rm -f results/*.csv
	rm -f results/*.png

-include $(DEPENDENCIES)