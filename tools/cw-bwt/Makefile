INCLUDE_FLAGS=-I../../extern/bitvector/include/ 

CLANG_CXXFLAGS=-Weverything -pedantic -Wno-c++98-compat -Wno-c++98-compat-pedantic
GCC_CXXFLAGS=-Wall -pedantic

ifndef STD
	STD=c++11
endif

ifndef OPTFLAGS
	ifneq (yes,$(DEBUG))		
		ARCHFLAGS=-march=native -mtune=generic
		ifneq (yes, $(ASSERTS))
			ASSERTSFLAGS=-DNDEBUG
		else
			ASSERTSFLAGS=
		endif
		OPTFLAGS=-Ofast -fstrict-aliasing $(ASSERTSFLAGS) $(ARCHFLAGS)
	else
		OPTFLAGS=-O0 -ggdb
	endif
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
../../data_structures/PartialSums.cpp \
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

	$(CXX) -o $(TARGET) -DDEBUG $(SOURCES)
	
clean:
	@rm -f $(TARGET)