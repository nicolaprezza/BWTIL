sa-to-bwt
===============
Welcome to the sa-to-bwt converter!

Authors: Nicola Prezza
mail: nicolapr@gmail.com

### Brief description

This tool can be used to build the BWT of a text file from its suffix array.

INPUT FORMAT: The SA file is assumed to be composed of n unsigned long int pointers (8 bytes each), where n is the text length. Of course, it is assumed that the input SA file is the suffix array of the input text file.

To check the correctness of the SA, use this tool to convert it into a BWT and then use bwt-check.

### Complexity

n = input length, s = alphabet size

SPACE:	n Bytes. Only the input text is stored in memory.

TIME:	O(n) steps.

### Execute

In the BWTIL/ directory, execute

> ./sa-to-bwt

to display info about the tool usage.
