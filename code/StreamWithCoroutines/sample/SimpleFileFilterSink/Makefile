.PHONY: all clean

include ../Makefile.common

PROGRAM = main

all: $(PROGRAM)

$(PROGRAM): $(PROGRAM).cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $(PROGRAM).cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(PROGRAM).o -o $@ 

clean:
	rm -fr $(PROGRAM).o $(PROGRAM)

