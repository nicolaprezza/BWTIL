count-runs
===============
Welcome to lz77!

Authors: Nicola Prezza
mail: nicolapr@gmail.com

### Brief description

This tool can be used to build the LZ77 parse of a text file. 2 versions of LZ77 have been implemented. The resulting parse can be output to screen in the form of pairs (position,phrase) or saved to file in the form of phrases separated by a separator. The program works building a dynamic compressed FM index, with a dynamic sampled SA to perform locate. RAM usage is zero-order compressed
and time is n log n 

INPUT FORMAT: simple ASCII-encoded text file.

### Execute

In the BWTIL/ directory, execute

> ./lz77

to display info about the tool usage.
