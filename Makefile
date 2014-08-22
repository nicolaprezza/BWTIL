INCLUDE_FLAGS=-I extern/bitvector/include/

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
		OPTFLAGS=-O0 -ggdb -g
	endif
endif

CCVER=$(shell $(CXX) --version)

ifneq (,$(findstring g++,$(CXX)))
	MY_CXXFLAGS=$(GCC_CXXFLAGS) $(CXXFLAGS)
else
	MY_CXXFLAGS=$(CLANG_CXXFLAGS) $(CXXFLAGS)
endif

SOURCES_CW_BWT=tools/cw-bwt/cw-bwt.cpp

SOURCES_SFM_INDEX=tools/sFM-index/sFM-index.cpp

SOURCES_DB_HASH=tools/dB-hash/dB-hash.cpp

SOURCES_BWT_CHECK=tools/bwt-check/bwt-check.cpp

SOURCES_BWT_TO_SA=tools/bwt-to-sa/bwt-to-sa.cpp

SOURCES_SA_TO_BWT=tools/sa-to-bwt/sa-to-bwt.cpp

SOURCES_BWT_INVERT=tools/bwt-invert/bwt-invert.cpp

SOURCES_TEST=tools/test/test.cpp

all: 

	@echo "Compiling with $(CXX)..."
	make clean
	make sFM-index
	make cw-bwt
	make dB-hash	
	make bwt-check
	make bwt-to-sa
	make sa-to-bwt
	make bwt-invert

sFM-index:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o sFM-index $(SOURCES_SFM_INDEX)

cw-bwt:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o cw-bwt $(SOURCES_CW_BWT)

dB-hash:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o dB-hash $(SOURCES_DB_HASH)

bwt-check:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o bwt-check $(SOURCES_BWT_CHECK)

bwt-to-sa:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o bwt-to-sa $(SOURCES_BWT_TO_SA)

sa-to-bwt:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o sa-to-bwt $(SOURCES_SA_TO_BWT)

bwt-invert:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o bwt-invert $(SOURCES_BWT_INVERT)

debug:

	@$(CXX) -std=$(STD) $(MY_CXXFLAGS) $(INCLUDE_FLAGS) $(OPTFLAGS) -o debug $(SOURCES_TEST)

testcases:

	./cw-bwt data/plain/sources.1MB data/bwt/sources.1MB.bwt
	./bwt-check data/bwt/sources.1MB.bwt data/plain/sources.1MB
	./bwt-invert data/bwt/sources.1MB.bwt data/plain/sources.1MB_copy
	./bwt-to-sa data/bwt/sources.1MB.bwt data/SA/sources.1MB.sa
	./sa-to-bwt data/SA/sources.1MB.sa data/plain/sources.1MB data/bwt/sources.1MB_copy.bwt
	./dB-hash build data/plain/sources.1MB 20
	./dB-hash search data/plain/sources.1MB.dbh "static unsigned long"
	./sFM-index build data/plain/sources.1MB
	./sFM-index search data/plain/sources.1MB.sfm "static unsigned long"

clean:

	rm -f dB-hash cw-bwt bwt-check bwt-to-sa sa-to-bwt bwt-invert sFM-index test
