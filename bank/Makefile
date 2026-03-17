CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2 -pthread -I include
BUILDDIR := build
SRC := src
TESTS := tests

BANK_LIB_OBJ := $(BUILDDIR)/Bank.o
SIM_OBJ := $(BUILDDIR)/main.o
SIMULATION_OBJ := $(BUILDDIR)/Simulation.o
CLI_OBJ := $(BUILDDIR)/Cli.o
TEST_OBJ := $(BUILDDIR)/bank_tests.o

SIM_TARGET := bank_sim
TEST_TARGET := bank_tests

.PHONY: all clean test run

all: $(SIM_TARGET) $(TEST_TARGET)

$(SIM_TARGET): $(BANK_LIB_OBJ) $(SIMULATION_OBJ) $(CLI_OBJ) $(SIM_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_TARGET): $(BANK_LIB_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILDDIR)/Bank.o: $(SRC)/Bank.cpp include/Bank.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/main.o: $(SRC)/main.cpp include/Cli.h include/Simulation.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/Simulation.o: $(SRC)/Simulation.cpp include/Simulation.h include/Bank.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/Cli.o: $(SRC)/Cli.cpp include/Cli.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/bank_tests.o: $(TESTS)/bank_tests.cpp include/Bank.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

run: $(SIM_TARGET)
	./$(SIM_TARGET)

clean:
	rm -rf $(BUILDDIR) $(SIM_TARGET) $(TEST_TARGET)
