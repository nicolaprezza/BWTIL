bwt-to-sa
===============
Welcome to the bwt-to-sa converter!

Authors: Nicola Prezza
mail: nicolapr@gmail.com

### Brief description

This tool can be used to build the suffix array of a text file from its BWT.

bwt-to-sa firstly indexes the bwt using a succinct wavelet-tree based data structure. Once the bwt is indexed, the tool extracts from it the SA pointers one by one, directly saving them to disk (the SA is not stored in RAM). In the indexed BWT, SA pointers are sampled every 'offset' positions of the text (this establishes a space-time tradeoff).

by default, offset = log n / log s (s being the alphabet size)

INPUT FORMAT: the bwt file is assumed to be a valid bwt of some file, with a 0x0 byte (text terminator) appearing only once inside it.

### Complexity

n = input length, s = alphabet size

SPACE:	O(n * log s + log n * n/offset) bits. With default parameters : O(n * log s) bits

TIME:	O(n * log s * offset). With default parameters: O(n log n)

ALTERNATIVE TRADEOFFS:

- specifying offset = 1, the space is O(n * log n) and time is linear O(n * log s)
- specifying offset = sqrt(log n)/log s, both space and time are O(n * sqrt(log n))

### Execute

In the BWTIL/ directory, execute

> ./bwt-to-sa

to display info about the tool usage.
