TARGET:=apqxx-test

CXXFLAGS?=-g -O2 -Wall -Wno-unknown-pragmas -std=c++17 \
	-DVERSION='"$(VERSION_STRING)"' \
	$(shell pkg-config --cflags libpq)
CXXINCFLAGS?=
LDFLAGS?=-pthread \
	-lboost_system \
	$(shell pkg-config --libs libpq) 
CXX?=g++

GIT_HASH := $(shell git log -1 --pretty=format:g%h)
GIT_DIRTY := $(shell git describe --all --long --dirty | grep -q dirty && echo 'dirty' || true)
GIT_TAG := $(shell git describe --exact-match 2>/dev/null || true)
VERSION_STRING := $(if $(GIT_TAG),$(GIT_TAG),$(GIT_HASH))$(if $(GIT_DIRTY), (dirty),)
OBJECTS=$(subst .cc,.o,$(shell ls *.cc))

$(TARGET): $(OBJECTS) 
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CXXINCFLAGS) -c $< -o $@
	$(CXX) -MM -MT $@ $(CXXFLAGS) $(CXXINCFLAGS) -c $< > $*.d

.PHONY: clean

clean:
	rm -f *.o *.d
	rm -f $(TARGET)

-include *.d
