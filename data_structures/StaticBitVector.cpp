//============================================================================
// Name        : StaticBitVector.cpp
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description :

 /*
 *   This class implements a bitvector with constant time support for rank and access queries
 *   space occupancy = n + n/D^2 * log n + n/D * log D^2 (D=24 in the default setting)
 */

//============================================================================


#include "StaticBitVector.h"

namespace bwtil {

StaticBitVector::StaticBitVector(ulint n) {

	this->n = n;

	w = ceil(log2(n));

	if(w<1) w=1;

	D = 63;

	/*
	 * bits are grouped in words of D bits.
	 * rank structures:
	 * 	1) one pointer of log n bits every D^2 positions (rank_ptrs_1)
	 * 	2) one pointer of log D^2 bits every D positions (rank_ptrs_2)
	 *
	 * 	space occupancy = n + n/D^2 * log n + n/D * log D^2
	 *
	 */

	bitvector = new WordVector( ceil((double)(n+1)/D) , D);// the bitvector
	rank_ptrs_1 = new WordVector( ceil((double)(n+1)/(D*D)) , w);//log n bits every D^2 positions
	rank_ptrs_2 = new WordVector( ceil((double)(n+1)/D) , 2*ceil(log2(D)));//2 log D bits every D positions

}

void StaticBitVector::computeRanks(){//compute rank structures basing on the content of the bitvector

	ulint nr_of_ones_global = 0;
	ulint nr_of_ones_local = 0;

	rank_ptrs_1->setWord(0,0);
	rank_ptrs_2->setWord(0,0);

	if(n==0)
		return;

	for(ulint i=0;i<n;i++){

		if((i+1)%(D*D)==0)
			nr_of_ones_local = 0;
		else
			nr_of_ones_local += bitAt(i);

		nr_of_ones_global += bitAt(i);

		if((i+1)%D==0)
			rank_ptrs_2->setWord((i+1)/D,nr_of_ones_local);

		if((i+1)%(D*D)==0)
			rank_ptrs_1->setWord((i+1)/(D*D),nr_of_ones_global);

	}

}

ulint StaticBitVector::size(){

	return bitvector->size() + rank_ptrs_1->size() + rank_ptrs_2->size();

}

string printWord(ulint x){

	string s;

	for(uint i=0;i<64;i++)
		s += (((x>>(63-i))&1)==0?'0':'1');

	return s;

}

//alternatives for popcount:

/*int popcnt(unsigned long int x){//no need for HW implementation

	x = x - ((x&0xAAAAAAAAAAAAAAAA)>>1);//groups of 2 bits
	x = (x&0x3333333333333333)+((x&0xCCCCCCCCCCCCCCCC)>>2);//groups of 4 bits
	x = (x&0x0F0F0F0F0F0F0F0F)+((x&0xF0F0F0F0F0F0F0F0)>>4);//groups of 8 bits

	x = x+(x>>32);//accumulate 8 counters of 8 bits in 4 counters of 8 bits
	x = x+(x>>16);//accumulate 4 counters of 8 bits in 2 counters of 8 bits
	x = x+(x>>8);//accumulate 2 counters of 8 bits in 1 counter of 8 bits

	return x&0x00000000000000FF;

}*/

//const-time popcnt (if available in hardware)
//#define popcnt(x) __builtin_popcount(x>>32)+__builtin_popcount(x&0x00000000FFFFFFFF);

#define popcnt(x) __builtin_popcountll(x)

ulint StaticBitVector::rank(ulint W, uint i){//number of 1's before position i (excluded) in the word W (of D bits)

	//warning: i must be <D

	ulint mask = (((ulint)1<<i)-1)<<(D-i);
	ulint masked = W & mask;

	return popcnt(masked);

}

ulint StaticBitVector::rank1(ulint i){//number of 1's before position i (excluded) in the bitvector

	return rank_ptrs_1->wordAt(i/(D*D)) + rank_ptrs_2->wordAt(i/D) + rank(bitvector->wordAt(i/D),i%D);

}

uint StaticBitVector::bitAt(ulint i){//get value of the i-th bit

	return bitvector->bitAt(i);

}

void StaticBitVector::setBit(ulint i, uint b){//set bit in position i

	bitvector->setBit(i,b);

}

void StaticBitVector::test(){

	for(uint i=0;i<rank_ptrs_1->length();i++)
		cout << rank_ptrs_1->wordAt(i) << ", ";

	cout << endl;
	for(uint i=0;i<rank_ptrs_2->length();i++)
		cout << rank_ptrs_2->wordAt(i) << ", ";


}

void StaticBitVector::freeMemory(){

	bitvector->freeMemory();
	rank_ptrs_1->freeMemory();
	rank_ptrs_2->freeMemory();

}


void StaticBitVector::saveToFile(FILE *fp){

	fwrite(&n, sizeof(ulint), 1, fp);
	fwrite(&w, sizeof(uint), 1, fp);
	fwrite(&D, sizeof(uint), 1, fp);

	bitvector->saveToFile(fp);
	rank_ptrs_1->saveToFile(fp);
	rank_ptrs_2->saveToFile(fp);

}

#define check_numBytes() if (numBytes == 0) { VERBOSE_CHANNEL << "Read 0 bytes when reading dB-hash file (StaticBitVector error)" << endl << flush; exit(1); }

void StaticBitVector::loadFromFile(FILE *fp){

	ulint numBytes;

	numBytes = fread(&n, sizeof(ulint), 1, fp);
	check_numBytes();
	numBytes = fread(&w, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&D, sizeof(uint), 1, fp);
	check_numBytes();

	bitvector = new WordVector();
	rank_ptrs_1 = new WordVector();
	rank_ptrs_2 = new WordVector();

	bitvector->loadFromFile(fp);
	rank_ptrs_1->loadFromFile(fp);
	rank_ptrs_2->loadFromFile(fp);

}

StaticBitVector::~StaticBitVector() {



}

} /* namespace data_structures */
