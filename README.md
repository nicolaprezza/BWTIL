BWTIL : the BWT Text-indexing Library
===============
Authors: Nicola Prezza, Nicola Gigante, and Alberto Policriti.
mail: nicolapr@gmail.com

### Brief introduction

The BWTIL library offers a set of tools, classes and functions designed to facilitate operations that involve manipulating the Burrows-Wheeler text transform (example: computing/inverting the BWT or checking its correctness) and indexes based on the BWT. The library is specifically designed to deal with large files (which means up to 2^64 characters) and implements efficient algorithms and data structures to offer good time-and-space performances. BWTIL comes with some sample programs and classes, among which you can find static and dynamic bitvectors, Huffman-compressed dynamic strings, partial sums, Huffman trees, static wavelet trees, the dB-hash data structure (ISAAC2014 paper: http://link.springer.com/chapter/10.1007%2F978-3-319-13075-0_13), a succinct wavelet-tree based FM index, and a novel BWT construction algorithm (LATA2015 paper: http://link.springer.com/chapter/10.1007/978-3-319-15579-1_46). The available executables (to date) are:

 * **dB-hash** : an implementation of the dB-hash data structure (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/dB-hash)
 
 * **sFM-index** : an implementation of a succinct (uncompressed) FM-index (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/sFM-index)

 * **cw-bwt** : a novel context-wise bwt construction algorithm (see https://github.com/nicolaprezza/BWTIL/tree/master/tools/cw-bwt)
 
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
