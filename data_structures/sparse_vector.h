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
// Name        : sparse_vector.h
// Author      : Nicola Prezza
// Version     : 1.0
// Description :

 /*
  * sparse vector: only some elements are nonzero. The zero elements waste only 1 bit each, while the
  * nonzero elements use all the space they need (which depends to the container implementation.)
  */

//============================================================================


#ifndef SPARSE_VECTOR_H_
#define SPARSE_VECTOR_H_

#include "../common/common.h"
#include "StaticBitVector.h"

namespace bwtil {

//sparse_vector requires the type of contained elements and the underlying container type (vector by default). The difference with the underlying Container type is that sparse_vector uses only 1 bit for the positions containing null elements (which must be specified in the declaration)
template <typename T, template <typename ...> class Container = vector> class sparse_vector{

	Container<T> container;//the underlying container
	StaticBitVector bitvector;//bitvector marking non-null elements
	ulint n,ones;//total size/number of non-null elements

public:

	sparse_vector(){};

	sparse_vector(vector<bool> &non_null_elements){

		n = non_null_elements.size();
		bitvector = StaticBitVector(non_null_elements);
		ones = bitvector.rank1(n);

		container = Container<T>(ones);

	}

	//access operator
	T operator[](ulint i){

		assert(i<n);

		// return 0 if the element is not marked. Works only if T is numeric!

		if(bitvector[i])
			return container[ bitvector.rank1(i) ];

		return 0;

	}

	//this method increments by 1 the element in position i. Of course, increment() must be implemented in the container
	void increment(ulint i){

		assert(i<n);

		//increment does nothing if performed on the zero elements
		if(bitvector[i])
			container.increment( bitvector.rank1(i) );

	}
	void set(ulint i, T value){

		assert(i<n);

		//set does nothing if performed on the zero elements
		if(bitvector[i])
			container.set( bitvector.rank1(i), value );

	}

	ulint size(){return n;}
	ulint nonzero_elements(){return ones;}


};


}

#endif /* SPARSE_VECTOR_H_ */
