CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
TARGET   := median-blur
SRCDIR   := src
BUILDDIR := build

SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs sdl2 2>/dev/null)
IMAGE_CFLAGS := $(shell pkg-config --cflags libpng 2>/dev/null)
IMAGE_LIBS   := $(shell pkg-config --libs libpng 2>/dev/null) -ljpeg
FONT_CFLAGS := $(shell pkg-config --cflags freetype2 2>/dev/null)
FONT_LIBS   := $(shell pkg-config --libs freetype2 2>/dev/null)

SRCS := $(SRCDIR)/App.cpp \
        $(SRCDIR)/Image.cpp \
        $(SRCDIR)/MedianFilter.cpp \
        $(SRCDIR)/OpenCLUtils.cpp \
        $(SRCDIR)/Renderer.cpp \
        $(SRCDIR)/main.cpp
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(SDL_LIBS) $(IMAGE_LIBS) $(FONT_LIBS) -lGL -lOpenCL

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(IMAGE_CFLAGS) $(FONT_CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)
