
CXX=$(shell sst-config --CXX)
CXXFLAGS=$(shell sst-config --ELEMENT_CXXFLAGS)
LDFLAGS=$(shell sst-config --ELEMENT_LDFLAGS)

all: sstbench.so install

sstbench.so: sstbench.cc
	$(CXX) $(CXXFLAGS) -O2 $(LDFLAGS) -o $@ $<

install:
	sst-register sstbench sstbench_LIBDIR=$(CURDIR)
	sst-register SST_ELEMENT_SOURCE sstbench=$(CURDIR)
	sst-register SST_ELEMENT_TESTS  sstbench=$(CURDIR)/../tests

clean:
	rm -f *.o sstbench.so
