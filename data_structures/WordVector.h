//============================================================================
// Name        : WordVector.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description :	this class implements a vector of n words, each of size w bits. Space: nw bits. Supports constant-time access to any word.
//				 	Alternatively, this structure can be used as a bitvector of size n*w bits, where each bit can be accessed and modified in
//					constant time and bits can be read in words of size w.
//
//============================================================================

#ifndef WORDVECTOR_H_
#define WORDVECTOR_H_

#include "../common/common.h"

namespace bwtil {

class WordVector {
public:

	WordVector(){};
	WordVector(ulint n, uint w);
	virtual ~WordVector();

	void setWord(ulint i, ulint W);//set word in position i to the value W
	ulint wordAt(ulint i);//get word in position i

	void setBit(ulint i, uint b);//set bit in position i
	uint bitAt(ulint i);//get value of the i-th bit
	uint bitAt(ulint word_nr,uint i){return bitAt(word_nr*w+i);}//get value of i-th bit in the word_nr-th word

	ulint length(){ return n; }

	void freeMemory();

	uint wordSize(){return w;};

	ulint size();//returns size of the structure in bits

	void saveToFile(FILE *fp);
	void loadFromFile(FILE *fp);

protected:

	ulint *v_64bit_words;//vector of 64-bits words. w-bits words are stored inside this vector.

	ulint n;//number of w-bits words
	ulint n64;//number of 64-bits words
	uint w;//number of bits in each word

	ulint max_value_size;//largest number that can be stored in a w-bits word = 2^w-1

};


} /* namespace data_structures */
#endif /* WORDVECTOR_H_ */
