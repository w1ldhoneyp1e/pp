CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2
SRCDIR := src
BUILDDIR := build
TARGET := piano

SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS := $(shell pkg-config --libs sdl2)

SRCS := $(SRCDIR)/main.cpp \
        $(SRCDIR)/PianoApp.cpp

OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(SDL_LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -I$(SRCDIR) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)
