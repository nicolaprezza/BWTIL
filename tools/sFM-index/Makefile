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

TARGET=../../sFM-index

SOURCES=\
sFM-index.cpp \
../../extern/sais.cpp \
../../data_structures/FileReader.cpp \
../../data_structures/StaticBitVector.cpp \
../../data_structures/succinctFMIndex.cpp \
../../data_structures/IndexedBWT.cpp \
../../data_structures/WaveletTree.cpp \
../../data_structures/WordVector.cpp

all: $(TARGET)

nonexistent:

$(TARGET): $(SOURCES)
	@echo "Compiling with $(CXX)..."
	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(OPTFLAGS) -o $(TARGET) $(SOURCES)

debug:

	$(CXX) -o $(TARGET) -DDEBUG $(SOURCES)
	
clean:
	@rm -f $(TARGET)
	
example-build:

	$(TARGET) build ../../data/plain/dna.1MB

example-search:

	$(TARGET) search ../../data/plain/dna.1MB.sfm ATCCATGTAGATATAACACAGCTATTTTCA