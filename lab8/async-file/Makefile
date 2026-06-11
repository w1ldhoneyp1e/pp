CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
TARGET   := async-file
SRCDIR   := src
BUILDDIR := build

SRCS := $(SRCDIR)/main.cpp
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/Task.h $(SRCDIR)/Dispatcher.h $(SRCDIR)/AsyncFile.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET) *.out
