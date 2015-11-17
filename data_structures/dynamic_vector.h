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
// Name        : dynamic_vector.h
// Author      : Nicola Prezza
// Version     : 1.0
/*
 * Description: a (semi) dynamic vector with access and insert-in-the-middle operations.
 * Substitute and delete are not yet implemented.
 *
 */

#ifndef DYNAMIC_VECTOR_H_
#define DYNAMIC_VECTOR_H_

#include "../common/common.h"
#include "bitvector.h"

namespace bwtil {

template<typename dynamic_bitvector_type>
class dynamic_vector{

public:

	/*
	 * default constructor. Sets a large max length (2^40) and width=64 bits
	 */
	dynamic_vector(){

		max_length = ulint(1)<<40;

	}

	/*
	 * constructor with max length of the vector and integer width
	 */
	dynamic_vector(ulint max_length, uint width){

		this->width=width;
		this->max_length=max_length;

		/*
		 * create a dynamic bitvector with max size the max number of bits of this dynamic vector
		 * and using the default word size for the nodes of the B tree
		 */
		bv = dynamic_bitvector_type(max_length*width);

	}

	/*
	 * get element in position i
	 */
	ulint const operator[](ulint i){

		return get(i);

	}

	/*
	 * get element in position i
	 */
	ulint get(ulint i){

		assert(i<current_length);

		ulint value = 0;

		for(uint j=0;j<width;j++)
			value = 2*value + bv[i*width+j];

		return value;

	}

	/*
	 * insert integer w in position i
	 */
	void insert(ulint i, ulint w){

		if(width<64)
			assert(w<( ulint(1)<<width  ));

		assert(current_length<max_length);
		assert(i<=current_length);

		for(uint j=0;j<width;j++){

			bool bit = w&ulint(1);
			w = w >> 1;

			bv.insert(i*width, bit);

		}

		current_length++;

	}

	/*
	 * current vector length
	 */
	ulint length(){return current_length;}
	ulint size(){return current_length;}

	/*
	 * maximum number of elements that can be stored in the vector
	 */
	ulint capacity(){return max_length;}

private:

	uint width=64;
	ulint max_length=0;
	ulint current_length=0;
	dynamic_bitvector_type bv;

};

typedef dynamic_vector<bitv> dynamic_vector_t;

}//namespace bwtil



#endif /* DYNAMIC_VECTOR_H_ */
