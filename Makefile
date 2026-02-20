CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -I make-archive
LDFLAGS =

SRCS = make-archive/main.cpp \
       make-archive/args/args.cpp \
       make-archive/archive/archive.cpp \
       make-archive/compression/compression.cpp \
       make-archive/compression_run/compression_run.cpp \
       make-archive/path/path.cpp \
       make-archive/temp/temp.cpp \
       make-archive/time/time.cpp
OBJS = $(SRCS:.cpp=.o)

make-archive/make-archive: $(OBJS)
	$(CXX) $(CXXFLAGS) -o make-archive/make-archive $(OBJS) $(LDFLAGS)

make-archive/%.o: make-archive/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean all
all: make-archive/make-archive

clean:
	rm -f make-archive/make-archive $(OBJS)
