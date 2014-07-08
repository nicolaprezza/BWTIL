#------------------------------------------------------------------------------

SOURCE=\
../../common/common.h \
cw-bwt.cpp \
../../data_structures/CumulativeCounters.cpp \
../../data_structures/DummyDynamicBitvector.cpp \
../../data_structures/BackwardFileReader.cpp \
../../data_structures/CwBWT.cpp \
../../data_structures/DynamicString.cpp \
../../data_structures/UnaryDynamicString.cpp \
../../data_structures/MultiaryDynamicString.cpp \
../../data_structures/HuffmanTree.cpp

BINARY=../../cw-bwt

#------------------------------------------------------------------------------

all: $(BINARY)

$(BINARY): $(SOURCE)

	g++ $(SOURCE) -o $(BINARY)

clean:

	rm -f $(BINARY)
