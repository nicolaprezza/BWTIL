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
// Description : Dynamic (only append) compressed BWT.

/*
 * Zero-order Huffman-compressed dynamic BWT. Initialized using absolute symbol frequencies (you need to know final text size and symbol frequencies).
 * Then, this class permits to update the BWT by left-extending the text, to do backward search, and to locate positions on the BWT.
 * Alphabet is numeric: {0,...,|freq|-1}, where freq is the input frequency vector
 *
 * Complexity is n log n H_0 for insert/access, sample_rate*log n for locate.
 *
 *
 */
//============================================================================

#ifndef DYNAMICBWT_H_
#define DYNAMICBWT_H_

#include "../common/common.h"
#include "DynamicString.h"
#include "dynamic_vector.h"

namespace bwtil {

template <typename dynamic_bitvector_type, typename dynamic_vector_type>
class DynamicBWT {
public:

	DynamicBWT(){}

	/*
	 * constructor. Parameters:
	 * freq = a vector containing absolute frequency of each character. Alphabet is {0,...,sigma-1}
	 * sample_rate = store a SA pointer every sample_rate positions.
	 */
	DynamicBWT(vector<ulint> freq, ulint sample_rate=0){

		this->sample_rate = sample_rate;

		n=0;
		for(ulint i=0;i<freq.size();i++)
			n += freq[i];

		n++;//add terminator

		sigma = freq.size();

		ds = DynamicString<dynamic_bitvector_type>(freq);

		current_size=1;//only terminator
		terminator_pos=0;

		//cumulative number of characters
		F = vector<ulint>(sigma,1);

		/*
		 * init structures for dynamic SA sampling
		 */
		if(sample_rate>0){

			number_of_samples = n/sample_rate + (n%sample_rate>0) + 1;//first text position always sampled
			sampled_positions = dynamic_bitvector_type(n);
			sa_samples = dynamic_vector_type(number_of_samples,number_of_bits(n));

			//position containing terminator is sampled.
			sampled_positions.insert(0,true);
			//position containing terminator is position number 0 in the text
			//(remember that positions are numbered from right to left)
			sa_samples.insert(0,0);

		}else{
			number_of_samples = 0;
		}

	}

	//extend an interval with a symbol. Return new interval
	pair<ulint, ulint> BS(pair <ulint,ulint> interval, symbol s){

		assert(interval.first <= size() and interval.second <= size());

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

	symbol const operator[](ulint i){
		return access(i);
	}

	//update BWT by left-extending text with character s
	ulint extend(symbol s){

		//insert in position terminator_pos character s
		ds.insert(s,terminator_pos);

		//update terminator position
		terminator_pos = F[s] + ds.rank(s,terminator_pos);

		//update F
		for(ulint i=s+1;i<sigma;i++)
			F[i]++;

		//update samples
		if(sample_rate>0){

			//check if this position has to be sampled
			//first text position is always sampled
			bool sampled = ((current_size%sample_rate)==0) or current_size==n-1;

			//always update bitvector
			sampled_positions.insert(terminator_pos,sampled);

			//update dynamic vector only if the position is sampled
			if(sampled)
				sa_samples.insert( sampled_positions.rank(terminator_pos,true), current_size );

		}

		current_size++;

		return terminator_pos;

	}

	/*
	 * LF function: map from position on BWT to position on F column
	 */
	ulint LF(ulint i){

		assert(i<current_size);

		symbol s = access(i);

		//if the symbol is terminato, then on F column position is 0
		if(s==sigma)
			return 0;

		return F[ s ] + rank(s,i);

	}

	/*
	 * locate: convert position on the F column to position on the text, where positions start from 0, with leftmost text character having position 0.
	 * Be aware that text is ended with terminator character
	 * parameter: i (position on the F column of the BWT)
	 * for this to work, you must have initialized the dynamic BWT with sample_rate>0
	 *
	 */
	ulint locate(ulint i){

		assert(sample_rate>0);

		if(i==terminator_pos)
			return 0;

		if(sampled_positions[i])
			return current_size - sa_samples[sampled_positions.rank(i,true)] -1;

		return locate(LF(i))+1;

	}

	/*
	 * locate_right: convert position on the F column to position on the text, where positions on the text proceed backwards with terminator having position 0.
	 * Be aware that text is ended with terminator character
	 * parameter: i (position on the F column of the BWT)
	 * for this to work, you must have initialized the dynamic BWT with sample_rate>0
	 *
	 */
	ulint locate_right(ulint i){

		assert(sample_rate>0);

		if(i==terminator_pos)
			return current_size-1;

		if(sampled_positions[i])
			return sa_samples[sampled_positions.rank(i,true)];

		return locate_right(LF(i)) - 1 ;

	}

	ulint getMaxLength(){ return n; }

private:

	DynamicString<dynamic_bitvector_type> ds;
	ulint n=0;//length of ds + 1 (terminator)
	ulint terminator_pos=0;

	ulint current_size=0;

	ulint sigma=0;//alphabet size and value of terminator

	vector<ulint> F;

	ulint sample_rate = 0;
	ulint number_of_samples=0;
	dynamic_bitvector_type sampled_positions;//mark with a 1 sampled positions
	dynamic_vector_type sa_samples;//sampled SA pointers. Note: positions start from the end of the text, with last character having position 1 and terminator having position 0

};

typedef DynamicBWT<bitv,dynamic_vector_t> dynamic_bwt_t;

} /* namespace bwtil */
#endif /* DYNAMICBWT_H_ */
