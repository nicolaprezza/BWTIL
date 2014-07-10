#------------------------------------------------------------------------------

SOURCE=\
../../common/common.h \
dB-hash.cpp \
../../extern/sais.cpp \
../../data_structures/StaticBitVector.cpp \
../../data_structures/DBhash.cpp \
../../data_structures/HashFunction.cpp \
../../data_structures/IndexedBWT.cpp \
../../extern/Random.cpp \
../../data_structures/WaveletTree.cpp \
../../data_structures/WordVector.cpp

BINARY=../../dB-hash

#------------------------------------------------------------------------------

all: $(BINARY)

$(BINARY): $(SOURCE)

	g++ $(SOURCE) -o $(BINARY)

clean:

	rm -f $(BINARY)

example-build:

	../../dB-hash build ../../data/50000.fasta 30

example-search:

	../../dB-hash search ../../data/50000.fasta.dbh aaatttataacctattgcaccatatactag
