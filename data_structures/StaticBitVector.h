//============================================================================
// Name        : StaticBitVector.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description :

 /*
 *   This class implements a bitvector with constant time support for rank and access queries
 *   space occupancy = n + n/D^2 * log n + n/D * log D^2 (D=24 in the default setting)
 */

//============================================================================



#ifndef STATICBITVECTOR_H_
#define STATICBITVECTOR_H_

#include "../common/common.h"

namespace bwtil {

class StaticBitVector {

public:

	StaticBitVector(){};

	StaticBitVector(vector<bool> * vb){

		this->n = vb->size();

		/*
		 * bits are grouped in words of D bits.
		 * rank structures:
		 * 	1) one pointer of log n bits every D^2 positions (rank_ptrs_1)
		 * 	2) one pointer of log D^2 bits every D positions (rank_ptrs_2)
		 *
		 * 	space occupancy = n + n/D^2 * log n + n/D * log D^2
		 *
		 */

		bitvector = bitview_t(n);

		for(ulint i=0;i<n;i++)
			bitvector.set(i,vb->at(i));

		rank_ptrs_1_size = n/(D*D) + 1;
		rank_ptrs_2_size = n/D + 1;

		rank_ptrs_1 = vector<uint64_t>( rank_ptrs_1_size,0 );//log n bits every D^2 positions
		rank_ptrs_2 = vector<uint16_t>( rank_ptrs_2_size,0 );//2 log D bits every D positions

		computeRanks();

	};

	ulint size(){

		return bitvector.size() + rank_ptrs_1.size() * 64 + rank_ptrs_2.size()*16;

	}

	inline ulint rank1(ulint i){//number of 1's before position i (excluded) in the bitvector

		assert(i<=n);
		assert(i/(D*D)<rank_ptrs_1_size);
		assert(i/D<rank_ptrs_2_size);
		assert((i/D)*D<=n);

		ulint remainder = ( ((i/D)*D)>=i?0:popcnt(bitvector.get((i/D)*D,i)) );

		return rank_ptrs_1[i/(D*D)] + rank_ptrs_2[i/D] + remainder;

	}

	inline ulint rank0(ulint i){

		assert(i<=n);

		return i-rank1(i);

	}//number of 0's before position i (excluded) in the bitvector

	inline uint bitAt(ulint i){//get value of the i-th bit

		assert(i<n);

		return bitvector.get(i);

	}

	void saveToFile(FILE *fp){

		fwrite(&n, sizeof(ulint), 1, fp);

		ulint bitvector_size = bitvector.size();

		fwrite(&rank_ptrs_1_size, sizeof(ulint), 1, fp);
		fwrite(&rank_ptrs_2_size, sizeof(ulint), 1, fp);

		fwrite(&bitvector_size, sizeof(ulint), 1, fp);

		fwrite(rank_ptrs_1.data(), sizeof(uint64_t), rank_ptrs_1_size, fp);
		fwrite(rank_ptrs_2.data(), sizeof(uint16_t), rank_ptrs_2_size, fp);

		save_bitview_to_file(bitvector,bitvector_size,fp);

	}

	void loadFromFile(FILE *fp){

		ulint numBytes;

		numBytes = fread(&n, sizeof(ulint), 1, fp);
		check_numBytes();

		ulint bitvector_size;

		numBytes = fread(&rank_ptrs_1_size, sizeof(ulint), 1, fp);
		check_numBytes();

		numBytes = fread(&rank_ptrs_2_size, sizeof(ulint), 1, fp);
		check_numBytes();

		numBytes = fread(&bitvector_size, sizeof(ulint), 1, fp);
		check_numBytes();

		rank_ptrs_1 = vector<uint64_t>(rank_ptrs_1_size,0);
		rank_ptrs_2 = vector<uint16_t>(rank_ptrs_2_size,0);

		numBytes = fread(rank_ptrs_1.data(), sizeof(uint64_t), rank_ptrs_1_size, fp);
		check_numBytes();

		numBytes = fread(rank_ptrs_2.data(), sizeof(uint16_t), rank_ptrs_2_size, fp);
		check_numBytes();

		bitvector = load_bitview_from_file(bitvector_size,fp);

	}

	ulint length(){return n;}

	ulint numberOf1(){return rank1(n);}
	ulint numberOf0(){return n-numberOf1();}

private:

	void computeRanks(){//compute rank structures basing on the content of the bitvector

		ulint nr_of_ones_global = 0;
		ulint nr_of_ones_local = 0;

		rank_ptrs_1[0]=0;
		rank_ptrs_2[0]=0;

		if(n==0)
			return;

		for(ulint i=0;i<n;i++){

			if((i+1)%(D*D)==0)
				nr_of_ones_local = 0;
			else
				nr_of_ones_local += bitAt(i);

			nr_of_ones_global += bitAt(i);

			if((i+1)%D==0)
				rank_ptrs_2[(i+1)/D]=nr_of_ones_local;

			if((i+1)%(D*D)==0)
				rank_ptrs_1[(i+1)/(D*D)]=nr_of_ones_global;

		}

	}


	ulint n;//length of the bitvector

	bitview_t bitvector;//the bits are stored in a bitvector, so that they can be accessed in blocks of size D
	vector<uint64_t> rank_ptrs_1;//rank pointers sampled every D^2 positions
	vector<uint16_t> rank_ptrs_2;//rank pointers sampled every D positions

	ulint rank_ptrs_1_size;
	ulint rank_ptrs_2_size;

	static const uint D = 64;//size of words

};

} /* namespace data_structures */
#endif /* BITVECTOR_H_ */
