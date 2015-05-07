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
// Name        : succinct_bitvector.h
// Author      : Nicola Prezza
// Version     : 1.0
// Description :

 /*
 *   This class implements a bitvector with constant time support for rank and access queries
 *   space occupancy = n + n/word_length^2 * log n + n/word_length * log word_length^2 (word_length=64)
 */

//============================================================================



#ifndef SUCCINCTBITVECTOR_H_
#define SUCCINCTBITVECTOR_H_

#include "../common/common.h"

namespace bwtil {

class succinct_bitvector {

public:

	/*
	 * argument: position i in the bitvector
	 * returns: bit in position i
	 * only access! the bitvector is static.
	 */
	inline uint operator[](ulint i){
		return at(i);
	}

	/*
	 * empty constructor. Initialize bitvector with length 0.
	 */
	succinct_bitvector(){};

	//build bitvector copying the content of the input vector of bool
	succinct_bitvector(vector<bool> &vb){

		n=0;
		global_rank1=0;
		local_rank1=0;

		for(ulint i=0;i<vb.size();i++)
			push_back(vb.at(i));

	};

	/*
	 * argument: a boolean b
	 * behavior: append b at the end of the bitvector.
	 */
	void push_back(bool b){

		if(n%word_length==0)
			updateVectors();

		global_rank1 += b;
		local_rank1 += b;

		insert(b);

		n++;

	}

	/*
	* returns: bit size of the data structure
	*/
	ulint size(){

		return bitvector.size()*word_length + rank_ptrs_1.size()*word_length + rank_ptrs_2.size()*16;

	}

	/*
	 * argument: position i in the bitvector, boolean b
	 * returns: number of bits equal to b before position i excluded
	 */
	ulint rank(ulint i, bool b=true){

		assert(i<=length());

		ulint ones = rank1(i);

		return b*ones + (1-b)*(i-ones) ;

	}

	//number of 1's before position i (excluded) in the bitvector
	inline ulint rank1(ulint i){

		ulint current_word = i/word_length;
		ulint remainder = i%word_length;

		if(i==n){
			assert(global_rank1<n);
			return global_rank1;
		}

		assert(i<=n);
		assert(i/(word_length_2)<rank_ptrs_1.size());
		assert(current_word<rank_ptrs_2.size());
		assert(current_word<bitvector.size());

		uint pop = 0;
		if(remainder>0)
			pop = popcnt( bitvector[current_word] >> ( word_length - remainder ) );

		ulint rank1 = rank_ptrs_1[i/word_length_2] + rank_ptrs_2[current_word] + pop;

		assert(rank1<=i);

		return rank1;

	}

	//number of 0's before position i (excluded) in the bitvector
	inline ulint rank0(ulint i){

		assert(i<=n);

		ulint rank0=i-rank1(i);

		assert(rank0<=i);

		return rank0;

	}

	inline uint at(ulint i){//get value of the i-th bit

		assert(i<n);

		return ( bitvector[i/word_length] >> ( (word_length-1) - (i%word_length) ) ) & ((ulint)1);

	}

	void saveToFile(FILE *fp){

		ulint rank_ptrs_1_size = rank_ptrs_1.size();
		ulint rank_ptrs_2_size = rank_ptrs_2.size();
		ulint bitvector_size = bitvector.size();

		fwrite(&n, sizeof(ulint), 1, fp);
		fwrite(&rank_ptrs_1_size, sizeof(ulint), 1, fp);
		fwrite(&rank_ptrs_2_size, sizeof(ulint), 1, fp);
		fwrite(&bitvector_size, sizeof(ulint), 1, fp);
		fwrite(&global_rank1, sizeof(uint64_t), 1, fp);
		fwrite(&local_rank1, sizeof(uint16_t), 1, fp);

		if(rank_ptrs_1_size>0) fwrite(rank_ptrs_1.data(), sizeof(uint64_t), rank_ptrs_1_size, fp);
		if(rank_ptrs_2_size>0) fwrite(rank_ptrs_2.data(), sizeof(uint16_t), rank_ptrs_2_size, fp);
		if(bitvector_size>0) fwrite(bitvector.data(), sizeof(uint64_t), bitvector_size, fp);

	}

	void loadFromFile(FILE *fp){

		ulint numBytes;
		ulint rank_ptrs_1_size;
		ulint rank_ptrs_2_size;
		ulint bitvector_size;

		numBytes = fread(&n, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		numBytes = fread(&rank_ptrs_1_size, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		numBytes = fread(&rank_ptrs_2_size, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		numBytes = fread(&bitvector_size, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		numBytes = fread(&global_rank1, sizeof(uint64_t), 1, fp);
		assert(numBytes>0);

		numBytes = fread(&local_rank1, sizeof(uint16_t), 1, fp);
		assert(numBytes>0);

		rank_ptrs_1 = vector<uint64_t>(rank_ptrs_1_size,0);
		rank_ptrs_2 = vector<uint16_t>(rank_ptrs_2_size,0);
		bitvector = vector<uint64_t>(bitvector_size,0);

		if(rank_ptrs_1_size>0){
			numBytes = fread(rank_ptrs_1.data(), sizeof(uint64_t), rank_ptrs_1_size, fp);
			assert(numBytes>0);
		}

		if(rank_ptrs_2_size>0){
			numBytes = fread(rank_ptrs_2.data(), sizeof(uint16_t), rank_ptrs_2_size, fp);
			assert(numBytes>0);
		}

		if(bitvector_size>0){
			numBytes = fread(bitvector.data(), sizeof(uint64_t), bitvector_size, fp);
			assert(numBytes>0);
		}

		numBytes++;//avoids "variable not used" warning

	}

	/*
	* returns: length of the bitvector (i.e. number of bits of the underlying bit sequence)
	*/
	ulint length(){return n;}

	ulint numberOf1(){return rank1(n);}
	ulint numberOf0(){return rank0(n);}

private:

	//called before inserting a bit in a position i multiple of word_length
	void updateVectors(){

		if(n%word_length_2==0){
			rank_ptrs_1.push_back(global_rank1);
			local_rank1=0;
		}

		rank_ptrs_2.push_back(local_rank1);

		bitvector.push_back(0);

	}

	//insert bit at the end (position n)
	inline void insert(bool b){

		//current word is the one at the end of bitvector
		//position in the word is n%word_length
		//moreover, insert bit only if it is 1 (otherwise nothing changes)

		if(b){

			bitvector[ bitvector.size()-1 ] |= ( ((ulint)1) << ( (word_length - 1) - (n%word_length)) );

			//if((global_rank1-1)%word_length_2 == 0)
				//sampled_select.push_back(rank_ptrs_1.size()-1);

		}

	}



	ulint n=0;//length of the bitvector

	vector<uint64_t> bitvector;//the bits are stored in a vector, so that they can be accessed in blocks of size word_length
	vector<uint64_t> rank_ptrs_1;//rank pointers sampled every word_length^2 positions
	vector<uint16_t> rank_ptrs_2;//rank pointers sampled every word_length positions
	//vector<uint64_t> sampled_select;//pointer to rank_ptrs_1 every word_length^2 ones


	uint64_t global_rank1=0;//rank1 up to position n excluded
	uint16_t local_rank1=0;//rank1 up to position n excluded from beginning of current block of size word_length^2

	static constexpr uint word_length = 64;//size of words
	static constexpr ulint word_length_2 = word_length*word_length;//square of size of words

};

} /* namespace data_structures */
#endif /* SUCCINCTBITVECTOR_H_ */
