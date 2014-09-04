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
 * DummyDynamicBitvector.h
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#ifndef DUMMYDYNAMICBITVECTOR_H_
#define DUMMYDYNAMICBITVECTOR_H_

#include "../common/common.h"

namespace bwtil {

class DummyDynamicBitvector {
public:

	DummyDynamicBitvector(){};

	//create a dynamic bitvector of size n. node_size is an unused parameter (legacy option)
	DummyDynamicBitvector(ulint n,size_t node_size=0) {

		this->n = n;
		bitvector = vector<bool>(n);
		waste = vector<bool>((n*2)/10);
		current_size = 0;

		node_size++;

	}

	bool access(ulint i){

		if(i>=current_size)
			cout << "WARNING: access in position " << i << " >= current size of the bitvector (" <<  current_size << ")\n";

		return bitvector.at(i);

	}

	void insert(ulint i, bool x){

		if(i>current_size){
			cout << "ERROR (DummyDynamicBitvector): insert in position " << i << " > current size of the bitvector (" <<  current_size << ")\n";
			exit(1);
		}

		for(ulint j = current_size;j>i;j--)
			bitvector.at(j) = bitvector.at(j-1);

		bitvector.at(i) = x;

		current_size++;

		if(current_size>n)
			cout << "WARNING (DummyDynamicBitvector): maximum size exceeded\n";


	}

	void print(){

		for(ulint i=0;i<size();i++)
			cout << bitvector.at(i);

		cout << endl;

	}

	ulint rank(ulint i, bool x){

		if(x==1)
			return rank1(i);

		return i-rank1(i);

	}

	uint height(){//height of the packed B-tree

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

		ulint r=0;

		for(ulint j=0;j<i;j++)
			r += bitvector.at(j);

		return r;

	}

	ulint current_size;
	ulint n;
	vector<bool> bitvector;
	vector<bool> waste;

};

} /* namespace compressed_bwt_construction */
#endif /* DUMMYDYNAMICBITVECTOR_H_ */
