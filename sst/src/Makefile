CXX=$(shell sst-config --CXX)
CXXFLAGS=$(shell sst-config --ELEMENT_CXXFLAGS)
LDFLAGS=$(shell sst-config --ELEMENT_LDFLAGS)
CPPFLAGS=-I./
OPTIMIZE_FLAGS=-O3

SOURCES := $(wildcard *.cc)
OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc))

all: libsstbench.so install

libsstbench.so: $(OBJS)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ *.o

sstbench.o: sstbench.cc sstbench.cc
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $<

install:
	sst-register sstbench sstbench_LIBDIR=$(CURDIR)
	sst-register SST_ELEMENT_SOURCE sstbench=$(CURDIR)

clean:
	rm -f *.o libsstbench.so

