/*
 * Counters.h
 *
 *  Created on: Jun 27, 2014
 *      Author: nicola
 */

#ifndef COUNTERS_H_
#define COUNTERS_H_

#include "../common/common.h"

namespace bwtil {

class Counters {

public:

	~Counters(){ delete [] words; }

	//the structure mantains one counter for each symbol in {0,1,...,sigma-1}.
	Counters(uint sigma, ulint n);//size of the alphabet and maximum number to be stored in a counter

	void increment(symbol s);//increment by 1 the counter in s

	//returns counter in s
	ulint at(symbol s);

	string toString();

	uint size(){return sigma;}

private:

	uint8_t sigma;
	uint8_t nr_of_words;
	uint8_t log2n;//number of bits of each counter
	uint8_t d;//number of counters per node = 64/log2(n)

	ulint * words;//64-bit words


};

} /* namespace compressed_bwt_construction */
#endif /* COUNTERS_H_ */
