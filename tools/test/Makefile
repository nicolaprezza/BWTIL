INCLUDE_FLAGS=-I../../extern/bitvector/include/

CLANG_CXXFLAGS=-Weverything -pedantic -Wno-c++98-compat -Wno-c++98-compat-pedantic
GCC_CXXFLAGS=-Wall -pedantic

ifndef STD
	STD=c++11
endif

ifndef OPTFLAGS
	OPTFLAGS=-Ofast -fstrict-aliasing
endif

CCVER=$(shell $(CXX) --version)

ifneq (,$(findstring g++,$(CXX)))
	MY_CXXFLAGS=$(GCC_CXXFLAGS) $(CXXFLAGS)
else
	MY_CXXFLAGS=$(CLANG_CXXFLAGS) $(CXXFLAGS)
endif

ifeq (c++,$(CXX))
	TARGET=../../test
else
	TARGET=../../test-$(CXX)
endif

SOURCES=\
test.cpp \
../../data_structures/DummyDynamicBitvector.cpp

all: $(TARGET)

nonexistent:

$(TARGET): $(SOURCES)
	@echo "Compiling test with $(CXX)..."
	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o $(TARGET) $(SOURCES)

clean:
	@rm -f ../../test ../../test-*