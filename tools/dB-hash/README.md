dB-hash
===============
Welcome to the dB-hash data structure!

Authors: Nicola Prezza and Alberto Policriti
mail: nicolapr@gmail.com

### Brief description

The dB-hash is a succinct data structure that simulates a hash table efficiently and in little space (O(n) bits instead of O(n log n) bits required by a standard hash) exploiting hash functions that are homomorphisms on de Bruijn graphs (de Bruijn hash functions). This space efficiency is paid with a slight slowdown of the lookup operation, which costs O(log m) (m being pattern length) instead of O(1) of a standard hash.

The dB-hash is particularly useful in approximate string matching under the Hamming distance, being the first succinct index offering expected O((log n)^k + m) time for a search with maximum Hamming distance k.

### Test

After compiling (see BWTIL/README), In the BWTIL/tools/dB-hash/ directory execute

> make example-build

to build a dB-hash with pattern length=30 on the file BWTIL/data/plain/dna.1MB . This will create the file BWTIL/data/plain/dna.1MB.dbh . After that, execute

> make example-search

to search the pattern "ATCCATGTAGATATAACACAGCTATTTTCA" (exact search) in the dB-hash just created.
