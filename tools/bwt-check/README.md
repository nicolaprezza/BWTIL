bwt-check
===============
Welcome to the bwt-checker!

Authors: Nicola Prezza
mail: nicolapr@gmail.com

### Brief description

This tool can be used to verify the correctness of bwt-construction algorithms. bwt-check takes as input a bwt file, the original file and reports whether the former is the correct bwt of the latter.

bwt-check operates building a (wavelet-tree based) succinct index (n+o(n) bytes) over the bwt file. This index is then used to navigate backwards the bwt in order to invert it and reconstruct the original text. This text is finally compared with the input text file.

INPUT FORMAT: the bwt file is assumed to be the bwt of the input file, with a 0x0 byte (text terminator) appearing only once inside the file.

### Execute

In the BWTIL/ directory, execute

> ./bwt-check

to display info about the tool usage.
