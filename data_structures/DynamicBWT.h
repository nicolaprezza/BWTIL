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

//============================================================================
// Name        : DynamicBWT.h
// Author      : Nicola Prezza
// Version     : 1.0
// Description : Dynamic (only append) compressed BWT. Note: at the moment, the class does not support locate (only count).

/*
 * Zero-order Huffman-compressed dynamic BWT. Initialized using absolute symbol frequencies (you need to know final text size and symbol frequencies).
 * Then, this class permits to update the BWT by left-extending the text and to do backward search (only count, not locate).
 *
 * Complexity is n log n for each operation.
 *
 *
 */
//============================================================================

#ifndef DYNAMICBWT_H_
#define DYNAMICBWT_H_

#include "../common/common.h"
#include "DynamicString.h"

namespace bwtil {

template <typename bitvector_type>
class DynamicBWT {
public:

	DynamicBWT(){}

	DynamicBWT(vector<ulint> freq){

		n=0;
		for(ulint i=0;i<freq.size();i++)
			n += freq[i];

		n++;//add terminator

		sigma = freq.size();

		ds = DynamicString<bitvector_type>(freq);

		current_size=1;//only terminator
		terminator_pos=0;

		//cumulative number of characters
		F = vector<ulint>(sigma,1);

	}

	//extend an interval with a char. Return new interval
	pair<ulint, ulint> BS(pair <ulint,ulint> interval, symbol s){

		interval.first = F[s] + rank(s,interval.first);
		interval.second = F[s] + rank(s,interval.second);

		return interval;

	}

	//return BWT position of terminator character
	ulint terminator_position(){return terminator_pos;};

	ulint size(){return current_size;}

	//adjusted rank: takes into account terminator_position()
	ulint rank(symbol s, ulint i){

		assert(i<n);
		assert(s<sigma);

		//positions are excluded
		if(i<=terminator_pos)
			return ds.rank(s,i);

		return ds.rank(s,i-1);

	}

	//adjusted acces: takes into account terminator_position()
	symbol access(ulint i){

		assert(i<n);

		if(i<terminator_pos)
			return ds.access(i);

		if(i==terminator_pos)
			return sigma;

		return ds.access(i-1);

	}

	//update BWT by left-extending text with character s
	void extend(symbol s){

		//insert in position terminator_pos character s
		ds.insert(s,terminator_pos);

		//update terminator position
		terminator_pos = F[s] + ds.rank(s,terminator_pos);

		//update F
		for(ulint i=s+1;i<sigma;i++)
			F[i]++;

		current_size++;

	}

	ulint getMaxLength(){ return n; }

private:

	DynamicString<bitvector_type> ds;
	ulint n;//length of ds + 1 (terminator)
	ulint terminator_pos;

	ulint current_size;

	ulint sigma;//alphabet size and value of terminator

	vector<ulint> F;

};

typedef DynamicBWT<bitv> dynamic_bwt_t;

} /* namespace data_structures */
#endif /* DYNAMICBWT_H_ */
