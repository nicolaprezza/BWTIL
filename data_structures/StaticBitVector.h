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

#include "WordVector.h"
#include "../common/common.h"

namespace bwtil {

class StaticBitVector {
public:
	StaticBitVector(){};
	StaticBitVector(ulint length);
	virtual ~StaticBitVector();

	uint bitAt(ulint i);//get value of the i-th bit
	void setBit(ulint i, uint b);//set bit in position i

	void computeRanks();//initializes rank structures basing on the content of the bitvector

	ulint rank1(ulint i);//number of 1's before position i (excluded) in the bitvector
	ulint rank0(ulint i){return i-rank1(i);};//number of 0's before position i (excluded) in the bitvector

	ulint length(){return n;}

	ulint numberOf1(){return rank1(n);}
	ulint numberOf0(){return n-numberOf1();}

	void freeMemory();

	void test();

	ulint size();//returns size of the structure in bits

	void saveToFile(FILE *fp);
	void loadFromFile(FILE *fp);

protected:

	ulint rank(ulint W, uint i);//number of 1's before position i (excluded) in the word W (of D bits)

	ulint n;//length of the bitvector
	uint w;//size of the word = log2 n

	WordVector * bitvector;//the bits are stored in a WordVector, so that they can be accessed in blocks of size D
	WordVector * rank_ptrs_1;//rank pointers sampled every D^2 positions
	WordVector * rank_ptrs_2;//rank pointers sampled every D positions

	uint D;//size of words

};

} /* namespace data_structures */
#endif /* BITVECTOR_H_ */
