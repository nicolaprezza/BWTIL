/*
 * CumulativeCounters.h
 *
 *  Created on: Jun 22, 2014
 *      Author: nicola
 *
 *      Description: maintains sigma cumulative counters; when incrementing counter i, all counters j>=i are incremented.
 *      Complexity: the structure is a packed B-tree. increment/access time = O( log_d(sigma), where d=w/log w and w is the word size (w=64) )
 *
 */

#ifndef CUMULATIVECOUNTERS_H_
#define CUMULATIVECOUNTERS_H_

#include "../common/common.h"

namespace bwtil {

class CumulativeCounters {

public:

	CumulativeCounters(){};

	//the structure mantains one counter for each symbol in {0,1,...,sigma-1}.
	CumulativeCounters(uint sigma, ulint n);//size of the alphabet and maximum number to be stored in a counter

	void increment(symbol s);//increment by 1 the counter in s, s+1, s+2, ...

	//number of symbols <s inserted. Eg: getCount(2) returns number of occurrencies of 0 and 1. getCount(0) returns always 0.
	ulint getCount(symbol s);

	string toString();

	void setBaseCounter(){base_counter=1;};

	uint bitSize(){//return size in bits

		return CHAR_BIT*(sizeof(this) + nr_of_nodes*sizeof(ulint));

	}

private:

	void incrementFrom(ulint * node, uint i);//increment counters in node by 1 starting from counter number i (from left)
	uint16_t parent(uint16_t n);//return parent of node n
	uint16_t child(uint16_t n, uint8_t i);//return child number i of node n
	uint8_t childNumber(uint16_t n);//return which children number is n in his parent
	ulint getCounterNumber(ulint n, uint i);//get value of the counter number i in the node n

	uint8_t sigma;
	uint8_t nr_of_leafs;
	uint8_t log2n;//number of bits of each counter
	uint8_t d;//number of counters per node = 64/log2(n)

	bool base_counter;//added to each count (1 only in the last context in the text, to count for one terminator)
	bool empty;

	uint16_t nr_of_nodes;

	ulint ones;//1^d in base 2^log2n
	ulint * nodes;//each node is a 64-bits word and stores d counters



};


} /* namespace compressed_bwt_construction */
#endif /* CUMULATIVECOUNTERS_H_ */
