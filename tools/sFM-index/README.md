succinct FM-index
===============
Welcome to the succinct FM-index data structure!

Authors: Nicola Prezza
mail: nicolapr@gmail.com

### Brief description

This is a version of the popular alphabet-friendly FM index, but without compression. The index occupies space close to the uncompressed text (succinct space).

### Test 

After compiling (see BWTIL/README), in the sFM-index/ directory execute

> make example-build

to build a succinct FM-index with pattern on the file BWTIL/data/plain/dna.1MB . This will create the file BWTIL/data/plain/dna.1MB.sfm . After that, execute

> make example-search

to search the pattern "ATCCATGTAGATATAACACAGCTATTTTCA" (exact search) in the index just created.

### Execute

In the BWTIL/ directory, execute

> ./sFM-index

to display info about the tool usage.
