CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
SRCDIR   := src
BUILDDIR := build
TARGET   := life

SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs   sdl2 2>/dev/null)
SDL_FOUND  := $(shell pkg-config sdl2 2>/dev/null && echo yes || echo no)

SRCS := $(SRCDIR)/main.cpp \
        $(SRCDIR)/Field.cpp \
        $(SRCDIR)/Generator.cpp \
        $(SRCDIR)/Stepper.cpp

ifeq ($(SDL_FOUND),yes)
    SRCS     += $(SRCDIR)/Visualizer.cpp
    CXXFLAGS += -DUSE_SDL $(SDL_CFLAGS)
    $(info SDL2 found — visualization enabled)
else
    $(info SDL2 not found — visualization disabled)
endif

OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread $(SDL_LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)
