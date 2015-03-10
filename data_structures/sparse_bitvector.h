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
  * sparse bitvector: static bitvector stored as a simple vector of integers (one for each 1).
  */

//============================================================================


#ifndef SPARSE_BITVECTOR_H_
#define SPARSE_BITVECTOR_H_

#include "../common/common.h"

namespace bwtil {

/*
 * the user can specify the type used in the underlying vector (this can be useful to save space if the maximum bitvector length is known beforehand)
 */
template<typename int_type = ulint> class sparse_bitvector{

public:

	/*
	 * empty constructor. Initialize bitvector with length 0.
	 */
	sparse_bitvector(){}

	/*
	 * argument: a boolean b
	 * behavior: append b at the end of the bitvector.
	 */
	void push_back(bool b){

		if(b){

			ones.push_back(length);
			length++;

		}else{

			length++;

		}

	}

	/*
	 * argument: position i in the bitvector
	 * returns: bit in position i
	 * only access! the bitvector is static.
	 */
	bool operator[](ulint i){

		assert(i<length);

		return std::binary_search(ones.begin(),ones.end(),i);

	}

	/*
	 * argument: position i in the bitvector
	 * returns: number of 1s before position i excluded
	 */
	ulint rank(ulint i){

		assert(i<=length);

		return std::lower_bound(ones.begin(),ones.end(),i) - ones.begin();

	}

	/*
	 * argument: integer i
	 * returns: position of the i-th one in the bitvector. i starts from 0
	 */
	ulint select(ulint i){

		assert(i<ones.size());

		return ones[i];

	}

	/*
	* returns: size of the bitvector
	*/
	ulint size(){return length;}

	/*
	 * returns: number of 1s in the bitvector
	 */
	ulint number_of_1(){

		return ones.size();

	}

private:

	ulint length=0;			//length of the bitvector
	vector<int_type> ones;	//position of each one

};


}

#endif /* SPARSE_BITVECTOR_H_ */
