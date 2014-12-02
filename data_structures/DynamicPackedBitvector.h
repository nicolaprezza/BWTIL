/*
 * DynamicPackedBitvector.h
 *
 *  Created on: Nov 28, 2014
 *      Author: nicola
 */

#ifndef DYNAMICPACKEDBITVECTOR_H_
#define DYNAMICPACKEDBITVECTOR_H_


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
 * DynamicPackedBitvector.h
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 *
 *  implements a dynamic bitvector of maximum size 2^16 -1
 *  efficient (O(1)) access and rank
 *  slow (O( n/w )) insert ( w=64 )
 *
 *  space: 1.19 n bits
 *
 */


#include "../common/common.h"

namespace bwtil {

class DynamicPackedBitvector {
public:

	DynamicPackedBitvector(){};

	//create a dynamic bitvector of size n. node_size is an unused parameter (legacy option)
	DynamicPackedBitvector(ulint n,size_t node_size=0) {

		this->n = n;

		n_words = n/64 + (n%64==0?0:1);

		rank_blocks_16 = vector<uint16_t>( n/256 + (n%256==0?0:1) );
		rank_blocks_8 = vector<uint8_t>( n_words );

		bitvector = vector<uint64_t>(n_words);
		current_size = 0;

	}

	inline bool access(ulint i){

		assert(i<current_size);

		return (bitvector[i/64]>>(63-(i%64))) & ((uint64_t)1);

	}

	void insert(ulint i, bool x){

		assert(i<=current_size);

		//bitvector.insert(bitvector.begin()+i, x);

		current_size++;

		assert(current_size<=n);

	}

	void print(){

		//cout << bitvector;

	}

	ulint rank(ulint i, bool x){

		assert(i<=current_size);

		return (x?rank1(i):i-rank1(i));

	}

	uint height(){//height of the packed B-tree. fake height

		uint ptr_size = ceil(log2(n+1)) +1;
		uint d = W/ptr_size;//keys per node
		uint b = sqrt(d);//worst-case fanout
		uint nr_of_leafs = n/W;
		if(b<=1) b=2;//fanout at least 2

		uint h = ceil(log2(nr_of_leafs)/log2(b));

		return h;

	}

	ulint size(){return current_size;};//current size

	ulint maxSize(){return n;};

	struct info_t {
		const size_t capacity;
		const size_t size;
		const size_t height;
		const size_t node_width;
		const size_t counter_width;
		const size_t pointer_width;
		const size_t degree;
		const size_t buffer;
		const size_t nodes;
		const size_t leaves;
	};

	info_t info() const {

		return {

			n,
			current_size,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0

		};

	};

private:

	static const uint W = 512;//word length

	ulint rank1(ulint i){

		//number of 1's before position i (excluded)

		return 0;

	}

	ulint current_size;
	ulint n;
	ulint n_words; //number of 64-bits words

	vector<uint64_t> bitvector;      // bitvector stored in blocks of 64 bits
	vector<uint16_t> rank_blocks_16; // every 256 bits
	vector<uint8_t> rank_blocks_8;   // every 64 bits

};

} /* namespace compressed_bwt_construction */


#endif /* DYNAMICPACKEDBITVECTOR_H_ */
