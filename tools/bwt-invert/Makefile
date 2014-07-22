#------------------------------------------------------------------------------

SOURCE=\
../../common/common.h \
bwt-invert.cpp \
../../data_structures/StaticBitVector.cpp \
../../data_structures/IndexedBWT.cpp \
../../data_structures/WaveletTree.cpp \
../../data_structures/WordVector.cpp \
../../data_structures/FileReader.cpp

BINARY=../../bwt-invert

#------------------------------------------------------------------------------

all: $(BINARY)

$(BINARY): $(SOURCE)

	g++ -O3 $(SOURCE) -o $(BINARY)

clean:

	rm -f $(BINARY)
