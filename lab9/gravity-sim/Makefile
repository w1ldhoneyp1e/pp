CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
TARGET   := gravity-sim
SRCDIR   := src
BUILDDIR := build

SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs sdl2 2>/dev/null)

SRCS := $(SRCDIR)/App.cpp \
        $(SRCDIR)/Camera.cpp \
        $(SRCDIR)/GravitySimulation.cpp \
        $(SRCDIR)/OpenCLUtils.cpp \
        $(SRCDIR)/Renderer.cpp \
        $(SRCDIR)/main.cpp
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(SDL_LIBS) -lGL -lOpenCL

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)
