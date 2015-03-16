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
#include "succinct_bitvector.h"

namespace bwtil {

template <class Container> class sparse_vector_reference{

	using value_type = typename Container::value_type;

public:

	sparse_vector_reference(Container &sv, ulint idx): _sv(sv), _idx(idx) {}

    operator value_type() {
        return _sv.get(_idx);
    }

	sparse_vector_reference const&operator=(value_type v) const {

		if(_sv.marked(_idx))
			_sv.set(_idx, v);

        return *this;
    }

	sparse_vector_reference const&operator++() const {

		if(_sv.marked(_idx))
			_sv.set(_idx, _sv[_idx]+1);

		return *this;

    }

	sparse_vector_reference const operator++(int) const {

		sparse_vector_reference copy(*this);

		if(_sv.marked(_idx))
			++(*this);

		return copy;

    }

private:

	Container &_sv;
	ulint _idx;

};

//sparse_vector requires the type of contained elements and the underlying container type (vector by default). The difference with the underlying Container type is that sparse_vector uses only 1 bit for the positions containing null elements (which must be specified in the declaration)
template <typename T, template <typename ...> class Container = vector> class sparse_vector{

	succinct_bitvector bitvector;//bitvector marking non-null elements
	ulint n,ones;//total size/number of non-null elements

	Container<T> container;//the underlying container

public:

	using value_type = T;

    using sparse_vector_ref = sparse_vector_reference<sparse_vector>;

	sparse_vector(){};

	sparse_vector(vector<bool> &non_null_elements){

		n = non_null_elements.size();
		bitvector = succinct_bitvector(non_null_elements);
		ones = bitvector.rank1(n);

		container = Container<T>(ones);

	}

	sparse_vector_ref operator[](ulint i){

		assert(i<n);

		return { *this, i };

	}

	T get(ulint i){

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
			container[ bitvector.rank1(i) ] = value ;

	}

	bool marked(ulint i){return bitvector[i];}

	ulint size(){return n;}
	ulint nonzero_elements(){return ones;}


};


}

#endif /* SPARSE_VECTOR_H_ */
