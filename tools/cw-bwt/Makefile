#------------------------------------------------------------------------------

SOURCE=\
../../common/common.h \
cw-bwt.cpp \
../../data_structures/CumulativeCounters.cpp \
../../data_structures/DummyDynamicBitvector.cpp \
../../data_structures/BackwardFileReader.cpp \
../../data_structures/FileReader.cpp \
../../data_structures/CwBWT.cpp \
../../data_structures/DynamicString.cpp \
../../data_structures/UnaryDynamicString.cpp \
../../data_structures/MultiaryDynamicString.cpp \
../../data_structures/HuffmanTree.cpp \
../../data_structures/ContextAutomata.cpp


BINARY=../../cw-bwt

#------------------------------------------------------------------------------

all: $(BINARY)

$(BINARY): $(SOURCE)

	g++ -O3 $(SOURCE) -o $(BINARY)

clean:

	rm -f $(BINARY)
