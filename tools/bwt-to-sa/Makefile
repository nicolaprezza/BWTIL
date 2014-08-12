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

TARGET=../../bwt-to-sa

SOURCES=\
bwt-to-sa.cpp \
../../data_structures/StaticBitVector.cpp \
../../data_structures/IndexedBWT.cpp \
../../data_structures/WaveletTree.cpp \
../../data_structures/WordVector.cpp \
../../data_structures/FileReader.cpp

all: $(TARGET)

nonexistent:

$(TARGET): $(SOURCES)
	@echo "Compiling with $(CXX)..."
	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(OPTFLAGS) -o $(TARGET) $(SOURCES)

debug:

	$(CXX) $(SOURCE) -o $(BINARY) -DDEBUG
	
clean:
	@rm -f $(TARGET)