CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -I make-archive
LDFLAGS =

SRCS = make-archive/main.cpp \
       make-archive/Args/Args.cpp \
       make-archive/Archive/Archive.cpp \
       make-archive/Compression/Compression.cpp \
       make-archive/CompressionRun/CompressionRun.cpp \
       make-archive/Path/Path.cpp \
       make-archive/Pipeline/Pipeline.cpp \
       make-archive/Temp/Temp.cpp \
       make-archive/Time/Time.cpp
OBJS = $(SRCS:.cpp=.o)

make-archive/make-archive: $(OBJS)
	$(CXX) $(CXXFLAGS) -o make-archive/make-archive $(OBJS) $(LDFLAGS)

make-archive/%.o: make-archive/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean all
all: make-archive/make-archive

clean:
	rm -f make-archive/make-archive $(OBJS)
