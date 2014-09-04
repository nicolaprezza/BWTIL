/*
 *  This file is part of BWTIL.
 *  Copyright (c) by
 *  Nicola Prezza <nicolapr@gmail.com>
 *
 *   BWTIL is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   BWTIL is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details (<http://www.gnu.org/licenses/>).
 */

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

		words = vector<ulint>(nr_of_words,0);

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

	vector<ulint> words;//64-bit words


};

} /* namespace compressed_bwt_construction */
#endif /* COUNTERS_H_ */
