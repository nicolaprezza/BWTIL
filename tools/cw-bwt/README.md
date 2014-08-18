cw-bwt: context-wise construction of the BWT in compressed space
===============
Welcome to the context-wise BWT construction algorithm (cw-bwt)!

Authors: Nicola Prezza, Nicola Gigante, and Alberto Policriti
mail: nicolapr@gmail.com

### Brief description

cw-bwt builds the BWT of a text (ASCII) file, using an amount of RAM comparable with the size of the compressed text. The algorithm exploits the partitioning induced by the length-k contexts to reduce the size of internal dynamic data structures used. As a result, for most text distributions of interest cw-bwt is optimal in both time and space. This is confirmed in practice: on a 2.4Ghz, core i7 PC, cw-bwt is able to build the bwt of the Human genome (3.2 billions of characters) in 4 hours and 30 minutes using only 992MB of RAM.

### Complexity

n = input length
Hk = k-th order entropy of the text, where k = log_s( n/log^2 n ) - 1 (Hk <= log s, where s is the alphabet size)

SPACE:	n * Hk + o(n * Hk) bits	(compressed succinct space)

TIME:

|    |      complexity      |  notes |
|----------|:-------------:|------:|
| expected/average |  O( n * Hk ) | (*) |
| worst-case |    O( n * log n * Hk )   |   (**) |

Notes: 

(*) Despite the worst case time of O(n log n Hk), by analysing the load of the internal data structures on a variety of texts (pizza&chilli repository) we can safely state that cw-bwt runs in average O(n Hk) time on most of the text of interest (i.e. are excluded only texts with extreme high redundancy). To check this, it is sufficient to run cw-bwt (e.g. on pizza&chilli sample texts) and verify indeed that the average packed B-tree height is (almost) always less than or equal to that expected from a uniform text (this info is provided as output). Since the expected complexity for a uniform random text is linear, it follows that also on most texts of interest cw-bwt runs in linear time.

(**) A factor log n * Hk from being optimal. The worst-case is a text with a lot of large (O(n)) length-k contexts; notice, however, that in this case the text is also highly compressible (Hk is small)

### Execute

In the BWTIL/ directory, execute

> ./cw-bwt

to display info about the tool usage.
