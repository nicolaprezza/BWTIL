BWTIL : the BWT Text-indexing Library
===============
Authors: Nicola Prezza, Nicola Gigante, and Alberto Policriti.
mail: nicolapr@gmail.com

### Brief introduction

The BWTIL library offers a set of tools, classes and functions designed to facilitate operations that involve manipulating the Burrows-Wheeler text transform (example: computing/inverting the BWT or checking its correctness) and indexes based on the BWT. The library is specifically designed to deal with large files (which means up to 2^64 characters) and implements efficient algorithms and data structures to offer good time-and-space performances. BWTIL comes with some sample programs and classes, among which you can find static and dynamic bitvectors, Huffman-compressed dynamic strings, partial sums, Huffman trees, static wavelet trees, the dB-hash data structure (paper published in proceedings ISAAC2014), a succinct wavelet-tree based FM index, and a novel BWT construction algorithm (paper accepted in LATA2015). The available executables (to date) are:

 * **dB-hash** (paper accepted in ISAAC2014!): an implementation of the dB-hash data structure (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/dB-hash)
 
 * **sFM-index** : an implementation of a succinct (uncompressed) FM-index (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/sFM-index)

 * **cw-bwt** (paper accepted in LATA2015!) : a novel context-wise bwt construction algorithm (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/cw-bwt)
 
 * **bwt-check** : check consistency of a BWT file (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/bwt-check)
 
 * **bwt-to-sa** : build the Suffix Array from a BWT file (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/bwt-to-sa)
 
 * **sa-to-bwt** : build the BWT from a suffix array file (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/sa-to-bwt)
 
 * **bwt-invert** : invert a BWT file to reconstruct the original text (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/bwt-invert)

 * **count-runs** : count number of equal-letter runs in a text file (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/count-runs)

 * **lz77** : Build the LZ77 parse (2 versions implemented) of the input text file. The parse can be output or saved to file. (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/lz77)

### Download

Since BWTIL includes extern git repositories as submodules, clone it using the --recursive option:

> git clone --recursive https://github.com/nicolaprezza/BWTIL

### Compile

The library has been extensively tested under linux using gcc 4.8.2 and clang++ 3.5. We use cmake to generate the Makefile:

To compile, firstly create and enter the bin/ directory

> mkdir bin; cd bin

Then, launch cmake as (default build type is release):

> cmake ..

Finally, build the executables:

> make

The above command creates the executables in the bin directory.

### Input formats

 * plain text files: ASCII-coded. However, the byte 0x0 must NOT appear inside the text since the algorithms use 0x0 as text terminator.
 * bwt files: ASCII-coded, with a UNIQUE 0x0 byte (terminator character) appearing somewhere inside the text. Be aware that, if the input bwt file is malformed, the programs will fail with a error message.
 * suffix array files: each SA address is stored as a 64-bit (8 byte) integer. Despite the fact that this coding may not be optimal for small texts, you can compress the SA files to reduce its size.

### Test

The folder BWTIL/data/plain/ contains some sample files to test the library. All files have been obtained taking the first 1M characters from the sample files of the pizza&chili text collection (http://pizzachili.dcc.uchile.cl/).

### Some examples

After compiling, in the main BWTIL/ directory run

> bin/cw-bwt data/plain/sources.1MB data/bwt/sources.1MB.bwt

To build the BWT of the text data/plain/sources.1MB. The BWT will be saved in data/bwt/sources.1MB.bwt

now run

> bin/bwt-check data/bwt/sources.1MB.bwt data/plain/sources.1MB

to test the correctness of the BWT file just created. Alternatively, run

> bin/bwt-invert data/bwt/sources.1MB.bwt data/plain/sources.1MB_copy

to invert the BWT and check that the two files data/plain/sources.1MB and data/plain/sources.1MB_copy are indeed identical (for example, using diff).

Now that we have the BWT of data/plain/sources.1MB, we can build its suffix array. Run

> bin/bwt-to-sa data/bwt/sources.1MB.bwt data/SA/sources.1MB.sa

The suffix array of data/plain/sources.1MB will be stored in data/SA/sources.1MB.sa

If we want to check the consistency of the SA, we can simply convert it back to a BWT with (for this we need also the original text file)

> bin/sa-to-bwt data/SA/sources.1MB.sa data/plain/sources.1MB data/bwt/sources.1MB_copy.bwt

And check with bwt-check the consistency of data/bwt/sources.1MB_copy.bwt against the plain text file data/plain/sources.1MB.

To search the occurrences of a pattern in a text file with the dB-hash data structure, first create the index (the file data/plain/sources.1MB.dbh will be created):

> bin/dB-hash build data/plain/sources.1MB 20

Where 20 is the pattern length. Then, to search for the occurrencies of the pattern of length 20 "static unsigned long", run

> bin/dB-hash search data/plain/sources.1MB.dbh "static unsigned long"

To search the occurrences of a pattern in a text file with the sFM-index data structure, first create the index (the file data/plain/sources.1MB.sfm will be created):

> bin/sFM-index build data/plain/sources.1MB

Then, to search for the occurrencies of the pattern "static unsigned long", run

> bin/sFM-index search data/plain/sources.1MB.sfm "static unsigned long"

