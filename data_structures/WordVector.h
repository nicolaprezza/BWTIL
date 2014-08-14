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

	WordVector(ulint n, uint w){

		if(w>63){
			cout << "Error: maximum admitted length of words is 63.\n";
			exit(1);
		}

		this->n=n;
		this->w=w;

		n64 = ceil(((double)n*w)/64);//number of 64-bits words

		if(n>0){

			v_64bit_words = new ulint[n64];

			for(ulint i=0;i<n64;i++)
				v_64bit_words[i]=0;//initialize all 0

		}

		max_value_size = ((ulint)1<<w)-1;

	}

	void setWord(ulint i, ulint W){

		if(i>n){
			cout << "Error: trying to access position " << i << " in a word-vector of length " << n << "\n";
			exit(1);
		}

		W = W & (((ulint)1<<w)-1);//mask out bits exceeding on the left

		//a w-bits word can span 2 64-bits words (w<64) or be contained in a single 64-bits word.
		ulint start_position = i*w;//included
		ulint end_position = i*w + w -1;//included

		ulint start_64bit_word = start_position/64;//position in v_64bit_words of the first bit
		//uint pos_in_first_64bit_word = start_position%64;

		ulint end_64bit_word = end_position/64;//position in v_64bit_words of the last bit
		uint pos_in_second_64bit_word = end_position%64;

		if(start_64bit_word==end_64bit_word){//the word is contained in a single 64-bit word

			//step 1: erase bits that have to be overwritten

			ulint mask = ((ulint)1<<w)-1;//11...1 (w times)

			if(pos_in_second_64bit_word<63)
				mask = mask << (63-pos_in_second_64bit_word);//shift left

			mask = ~mask;//complement

			v_64bit_words[start_64bit_word] &= mask; // reset bits that will be overwritten

			//step 2: write W

			if(pos_in_second_64bit_word<63)
				W = W << (63-pos_in_second_64bit_word);//shift left the word W

			v_64bit_words[start_64bit_word] |= W;

		}else{//the word spans two 64-bit words

			uint prefix_length = 64 - start_position;//length of prefix of W copied at the end of the first word
			uint suffix_length = w - prefix_length;//length of suffix of W copied at the beginning of the second word

			//step 1: erase bits that have to be overwritten

			ulint mask1 = ((ulint)1<<prefix_length)-1;
			mask1 = ~mask1;//111...111 0^prefix_length

			ulint mask2 = ((ulint)1<<suffix_length)-1;
			mask2 = mask2 << (64-suffix_length);
			mask2 = ~mask2;//0^suffix_length 111111...1111

			v_64bit_words[start_64bit_word] &= mask1; // reset bits that will be overwritten
			v_64bit_words[end_64bit_word] &= mask2; // reset bits that will be overwritten

			//step 2: write W

			v_64bit_words[start_64bit_word] |= (W>>suffix_length);//write prefix
			v_64bit_words[end_64bit_word] |= (W<< (64-suffix_length) );//write prefix


		}

	}

	ulint wordAt(ulint i){

		if(i>n){
			cout << "Error: trying to access position " << i << " in a word-vector of length " << n << "\n";
			exit(1);
		}

		//a w-bits word can span 2 64-bits words (w<64) or be contained in a single 64-bits word.
		ulint start_position = i*w;//included
		ulint end_position = i*w + w -1;//included

		ulint start_64bit_word = start_position/64;//position in v_64bit_words of the first bit
		//uint pos_in_first_64bit_word = start_position%64;

		ulint end_64bit_word = end_position/64;//position in v_64bit_words of the last bit
		uint pos_in_second_64bit_word = end_position%64;

		ulint result=0;

		if(start_64bit_word==end_64bit_word){//the word is contained in a single 64-bit word

			ulint mask = ((ulint)1<<w)-1;//11...1 (w times)

			if(pos_in_second_64bit_word<63)//if 64-bit word has to be right-shifted
				result = v_64bit_words[start_64bit_word] >> (63-pos_in_second_64bit_word);//shift
			else//word finishes exactly at the end of the 64-bits word
				result = v_64bit_words[start_64bit_word];

			result = result & mask;

		}else{//the word spans two 64-bit words

			uint prefix_length = 64 - start_position;//length of suffix of word 1 copied as prefix of the result
			uint suffix_length = w - prefix_length;//length of prefix of word 2 copied as suffix of the result

			//read prefix from word 1
			ulint mask1 = ((ulint)1<<prefix_length)-1; // 000...000 1^prefix_length
			result = (v_64bit_words[start_64bit_word] & mask1)<<suffix_length;

			//add suffix from word 2
			result |= ( v_64bit_words[end_64bit_word] >> (64-suffix_length) );

		}

		return result;

	}

	void setBit(ulint i, uint b){//set bit in position i

		if(i>n*w){
			cout << "Error: trying to access bit number " << i << " in a bit-vector of length " << n*w << " bits.\n";
			exit(1);
		}
		if(b>1){
			cout << "Error: " << b << " is not an admissible value for a bit.\n";
			exit(1);
		}

		ulint word_nr = i/64;
		uint pos = i%64;

		ulint mask = 1;
		mask = mask << (63-pos);

		mask = ~mask;

		v_64bit_words[word_nr] &= mask;//reset bit

		ulint W = (ulint)b;
		W = W << (63-pos);

		v_64bit_words[word_nr] |= W;

	}

	inline uint bitAt(ulint i){

		if(i>n*w){
			cout << "Error: trying to access bit number " << i << " in a bit-vector of length " << n*w << " bits.\n";
			exit(1);
		}

		ulint word_nr = i/64;
		uint pos = i%64;

		return (v_64bit_words[word_nr] >> (63-pos)) & 1;

	}

	void freeMemory(){
		if(n>0)
			delete [] v_64bit_words;
	}

	ulint size(){//returns size of the structure in bits

		return n64*64;

	}

	void saveToFile(FILE *fp){

		fwrite(&n, sizeof(ulint), 1, fp);
		fwrite(&n64, sizeof(ulint), 1, fp);
		fwrite(&w, sizeof(uint), 1, fp);
		fwrite(&max_value_size, sizeof(ulint), 1, fp);
		fwrite(v_64bit_words, sizeof(ulint), n64, fp);

	}

	void loadFromFile(FILE *fp){

		ulint numBytes;

		numBytes = fread(&n, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&n64, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&w, sizeof(uint), 1, fp);
		check_numBytes();
		numBytes = fread(&max_value_size, sizeof(ulint), 1, fp);
		check_numBytes();
		v_64bit_words = new ulint[n64];
		numBytes = fread(v_64bit_words, sizeof(ulint), n64, fp);
		check_numBytes();

	}

	inline uint bitAt(ulint word_nr,uint i){return bitAt(word_nr*w+i);}//get value of i-th bit in the word_nr-th word

	ulint length(){ return n; }

	uint wordSize(){return w;};

private:

	ulint *v_64bit_words;//vector of 64-bits words. w-bits words are stored inside this vector.

	ulint n;//number of w-bits words
	ulint n64;//number of 64-bits words
	uint w;//number of bits in each word

	ulint max_value_size;//largest number that can be stored in a w-bits word = 2^w-1

};


} /* namespace data_structures */
#endif /* WORDVECTOR_H_ */
