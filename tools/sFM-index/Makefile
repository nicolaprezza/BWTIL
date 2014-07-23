#------------------------------------------------------------------------------

SOURCE=\
../../common/common.h \
sFM-index.cpp \
../../extern/sais.cpp \
../../data_structures/FileReader.cpp \
../../data_structures/StaticBitVector.cpp \
../../data_structures/succinctFMIndex.cpp \
../../data_structures/IndexedBWT.cpp \
../../data_structures/WaveletTree.cpp \
../../data_structures/WordVector.cpp

BINARY=../../sFM-index

#------------------------------------------------------------------------------

all: $(BINARY)

$(BINARY): $(SOURCE)

	g++ -O3 $(SOURCE) -o $(BINARY)

clean:

	rm -f $(BINARY)

example-build:

	../../sFM-index build ../../data/plain/dna.1MB

example-search:

	../../sFM-index search ../../data/plain/dna.1MB.sfm ATCCATGTAGATATAACACAGCTATTTTCA
