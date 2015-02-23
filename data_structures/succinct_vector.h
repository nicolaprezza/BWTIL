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
// Name        : succinct_vector.h
// Author      : Nicola Prezza
// Version     : 1.0
// Description :

 /*
  * succinct vector: a vector of integers that uses the minimum number of bits to represent each integer. Moreover, updates are supported.
  */

//============================================================================

#ifndef SUCCINCT_VECTOR_H_
#define SUCCINCT_VECTOR_H_

#include "../common/common.h"
#include "../extern/bitvector/include/packed_view.h"

namespace bwtil {

template <class Container> class succinct_vector_reference{

	using value_type = typename Container::value_type;

public:

	succinct_vector_reference(Container &sv, ulint idx): _sv(sv), _idx(idx) {}

    operator value_type() {
        return _sv.get(_idx);
    }

    succinct_vector_reference const&operator=(value_type v) const {

		_sv.set(_idx, v);

        return *this;
    }

    succinct_vector_reference const&operator++() const {

		_sv.increment(_idx);

		return *this;

    }

    succinct_vector_reference const operator++(int) const {

    	succinct_vector_reference copy(*this);

		++(*this);

		return copy;

    }

private:

	Container &_sv;
	ulint _idx;

};

//template parameter: numeric (unsigned) type
template <typename T = ulint> class succinct_vector{

    static_assert(std::numeric_limits<T>::is_integer, "succinct_vector can be only instantiated with integer types");

    vector<packed_view<vector> > packed_views;
    ulint number_of_packed_views;
    uint field_width;//default number of bits reserved to each element in the succinct array
    ulint packed_view_size;//size of each packed view
    ulint n;

    uint bitlength(uint x){

    	uint bits = 0;

    	while(x>0){
    		x = x >> 1;
    		bits++;
    	}

    	return bits + (bits==0);
    }

public:

    using value_type = T;
    using succinct_vector_ref = succinct_vector_reference<succinct_vector>;

    succinct_vector(){};

    succinct_vector(ulint n, uint field_width = 1, ulint packed_view_size = 256){

    	number_of_packed_views = n/packed_view_size + (n%packed_view_size != 0);

    	this->n = n;
    	this->field_width = field_width;
    	this->packed_view_size = packed_view_size;

    	packed_views = vector<packed_view<vector> >(number_of_packed_views);

    	for(ulint i=0;i<number_of_packed_views;++i)
    		packed_views[i] = packed_view<vector>(field_width, packed_view_size);



    }

	succinct_vector_ref operator[](ulint i){

		assert(i<n);
		return { *this, i };

	}

	ulint get(ulint i){

		assert(i<n);
		return packed_views[i/packed_view_size][i%packed_view_size];

	}

	//set field number i to the specified value. This procedure resizes the corresponding packed_view if the new value cannot be stored in that number of bits.
	void set(ulint i, T value){

		assert(i<n);

		uint current_width = packed_views[i/packed_view_size].width();
		uint max_value = (ulint(1) << current_width) - 1;

		if(value > max_value){//resize

			vector<T> values = vector<T>(packed_view_size);

			for(ulint j=0;j<packed_view_size;++j)
				values[j] = packed_views[i/packed_view_size][j];

			values[i%packed_view_size] = value;

			packed_views[i/packed_view_size] = packed_view<vector>( bitlength(value) , packed_view_size);

			for(ulint j=0;j<packed_view_size;++j)
				packed_views[i/packed_view_size][j] = values[j];

		}else{
			packed_views[i/packed_view_size][i%packed_view_size] = value;
		}

	}

	void increment(ulint i){

		assert(i<n);

		uint current_width = packed_views[i/packed_view_size].width();
		uint max_value = (ulint(1) << current_width) - 1;
		uint current_value = packed_views[i/packed_view_size][i%packed_view_size];

		if(current_value+1 > max_value){//resize

			vector<T> values = vector<T>(packed_view_size);

			for(ulint j=0;j<packed_view_size;++j)
				values[j] = packed_views[i/packed_view_size][j];

			values[i%packed_view_size]++;

			packed_views[i/packed_view_size] = packed_view<vector>(current_width+1, packed_view_size);

			for(ulint j=0;j<packed_view_size;++j)
				packed_views[i/packed_view_size][j] = values[j];

		}else{
			packed_views[i/packed_view_size][i%packed_view_size] = packed_views[i/packed_view_size][i%packed_view_size] + 1;
		}

	}

	ulint size(){ return n;}

};

}

#endif /* SUCCINCT_VECTOR_H_ */
