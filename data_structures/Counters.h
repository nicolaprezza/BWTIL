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
	Counters(uint sigma, ulint n){

		if(n==0)//empty counters
			return;

		this->sigma=sigma;

		log2n = ceil(log2(n+1));

		uint w=64;
		d = w/log2n;

		nr_of_words = sigma/d + (sigma%d==0?0:1);//words storing the counters

		words = new ulint[nr_of_words];
		for(uint i=0;i<nr_of_words;i++)//reset all counters
			words[i]=0;

	}

	void increment(symbol s){//increment by 1 the counter in s

		uint word_nr = s/d;
		uint offset = s%d;

		words[word_nr] += ((ulint)1)<<((d-offset-1)*log2n);

	}

	ulint at(symbol s){

		uint word_nr = s/d;
		uint offset = s%d;
		ulint MASK = (((ulint)1)<<log2n)-1;

		return (words[word_nr]>>((d-offset-1)*log2n)) & MASK;

	}

	string toString(){

		stringstream ss;

		for(uint i=0;i<sigma;i++)
			ss << at(i) << " ";

		return ss.str();

	}


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
