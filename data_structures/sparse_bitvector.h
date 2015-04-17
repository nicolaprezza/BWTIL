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
  * sparse bitvector: static bitvector stored as a simple vector of integers (one for each 1). Operatins implemented through binary search.
  */

//============================================================================


#ifndef SPARSE_BITVECTOR_H_
#define SPARSE_BITVECTOR_H_

#include "../common/common.h"

namespace bwtil {

/*
 * the user can specify the type used in the underlying vector (this can be useful to save space if the maximum bitvector length is known beforehand)
 */
template<typename word_type = ulint> class sparse_bitvector{

public:

	/*
	 * empty constructor. Initialize bitvector with length 0.
	 */
	sparse_bitvector(){}

	/*
	 * argument: a vector of booleans b
	 * behavior: create sparse bitvector with content of bb
	 */
	sparse_bitvector(vector<bool> b){

		for(auto bb : b)
			push_back(bb);

	}

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
	 * argument: position i in the bitvector, boolean b
	 * returns: number of bits equal to b before position i excluded
	 */
	ulint rank(ulint i, bool b=true){

		assert(i<=length);

		ulint n1 = std::lower_bound(ones.begin(),ones.end(),i) - ones.begin();

		return b*n1 + (1-b)*(i-n1) ;

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

	/* serialize the structure to the ostream
	 * \param out	 the ostream
	 */
	ulint serialize(std::ostream& out){

		ulint w_bytes = 0;

		uint8_t word_s = sizeof(word_type);
		word_type ones_length = ones.size();

		out.write((char *)&word_s, sizeof(uint8_t));
		out.write((char *)&ones_length, word_s);
		out.write((char *)&length, sizeof(ulint));

		w_bytes += sizeof(uint8_t)+word_s+sizeof(ulint);

		out.write((char *)ones.data(), ones_length*word_s);
		w_bytes += ones_length*word_s;

		return w_bytes;

	}

	/* load the structure from the istream
	 * \param in the istream
	 */
	void load(std::istream& in) {

		uint8_t word_s;
		word_type ones_length;

		in.read((char *)&word_s, sizeof(uint8_t));

		assert(word_s==sizeof(word_type) and "Error: mismatching word lengths while loading sparse bitvector.");

		in.read((char *)&ones_length, word_s);
		in.read((char *)&length, sizeof(ulint));

		ones = vector<word_type>(ones_length);

		in.read((char *)ones.data(), ones_length*word_s);

	}



private:

	ulint length=0;			//length of the bitvector
	vector<word_type> ones;	//position of each one

};


}

#endif /* SPARSE_BITVECTOR_H_ */
