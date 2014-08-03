#------------------------------------------------------------------------------

SOURCE=\
../../common/common.h \
dB-hash.cpp \
../../extern/sais.cpp \
../../data_structures/StaticBitVector.cpp \
../../data_structures/DBhash.cpp \
../../data_structures/HashFunction.cpp \
../../data_structures/IndexedBWT.cpp \
../../data_structures/WaveletTree.cpp \
../../data_structures/WordVector.cpp

BINARY=../../dB-hash

#------------------------------------------------------------------------------

all: $(BINARY)

$(BINARY): $(SOURCE)

	g++ -O3 $(SOURCE) -o $(BINARY)

clean:

	rm -f $(BINARY)

example-build:

	../../dB-hash build ../../data/plain/dna.1MB 30

example-search:

	../../dB-hash search ../../data/plain/dna.1MB.dbh ATCCATGTAGATATAACACAGCTATTTTCA
