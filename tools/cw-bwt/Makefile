INCLUDE_FLAGS=-I../../extern/bitvector/include/ 

CLANG_CXXFLAGS=-Weverything -pedantic -Wno-c++98-compat -Wno-c++98-compat-pedantic
GCC_CXXFLAGS=-Wall -pedantic

ifndef STD
	STD=c++11
endif

ifndef OPTFLAGS
	OPTFLAGS=-Ofast -fstrict-aliasing -march=native -mtune=generic
endif

CCVER=$(shell $(CXX) --version)

ifneq (,$(findstring g++,$(CXX)))
	MY_CXXFLAGS=$(GCC_CXXFLAGS) $(CXXFLAGS)
else
	MY_CXXFLAGS=$(CLANG_CXXFLAGS) $(CXXFLAGS)
endif

TARGET=../../cw-bwt

SOURCES=\
cw-bwt.cpp \
../../data_structures/CumulativeCounters.cpp \
../../data_structures/DummyDynamicBitvector.cpp \
../../data_structures/BackwardFileReader.cpp \
../../data_structures/FileReader.cpp \
../../data_structures/CwBWT.cpp \
../../data_structures/DynamicString.cpp \
../../data_structures/HuffmanTree.cpp \
../../data_structures/ContextAutomata.cpp

all: $(TARGET)

nonexistent:

$(TARGET): $(SOURCES)
	@echo "Compiling with $(CXX)..."
	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o $(TARGET) $(SOURCES)

debug:

	$(CXX) $(SOURCE) -o $(BINARY) -DDEBUG
	
clean:
	@rm -f $(TARGET)