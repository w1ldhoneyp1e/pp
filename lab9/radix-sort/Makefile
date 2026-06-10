CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
TARGET   := radix-sort
SRCDIR   := src
BUILDDIR := build

SRCS := $(SRCDIR)/OpenCLUtils.cpp \
        $(SRCDIR)/RadixSorter.cpp \
        $(SRCDIR)/main.cpp
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lOpenCL

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)
