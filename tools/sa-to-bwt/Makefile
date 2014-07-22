#------------------------------------------------------------------------------

SOURCE=\
../../common/common.h \
sa-to-bwt.cpp \
../../data_structures/FileReader.cpp

BINARY=../../sa-to-bwt

#------------------------------------------------------------------------------

all: $(BINARY)

$(BINARY): $(SOURCE)

	g++ -O3 $(SOURCE) -o $(BINARY)

clean:

	rm -f $(BINARY)
