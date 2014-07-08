//============================================================================
// Name        : HashFunction.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : de Bruijn hash function to be used in the dB-hash.
//============================================================================

#ifndef HASHFUNCTION_H_
#define HASHFUNCTION_H_

#include "../extern/Random.h"
#include "../common/common.h"
#include <sstream>
#include <fstream>

namespace bwtil {

class HashFunction {
public:

	class SetZIterator{

	public:

		SetZIterator(HashFunction * f){
			Z = f->Z;
			Z_size = f->Z_size;
			current_errors=0;
			index=0;
		}

		ulint nextElement();
		bool hasNext(){return current_errors<=radius; };
		uint getNextErrors(){return current_errors;};//return number of errors of element to be extracted

	private:

		uint current_errors;
		ulint index;
		ulint **Z;//set Z
		uint *Z_size;//size of Z(i)

	};


	HashFunction();

	//n = text length, m= pattern length. w is automatically calculated as w = log_b(mn), where b = base (which depends on type)
	HashFunction(ulint n, ulint m, hash_type type=DEFAULT, bool verbose = false);

	//reads the file to detect optimal base (=alphabet cardinality) for the hash function
	HashFunction(ulint m, const char * file_path, bool verbose = false);

	Random * random;

	// ----- fingerprint computation

	ulint hashValue(unsigned char *P);//compute fingerprint of pattern P of length m.
	ulint hashValue(string P);//compute fingerprint of pattern P of length m.

	unsigned char *hashValue(unsigned char *P, ulint n);//compute fingerprint of pattern P of length n>=m. (possible since function is de Bruijn). returns text of length n-m+w

	unsigned char *hashValueRemapped(unsigned char *P, ulint n);//compute fingerprint of pattern P of length n>=m, then add 1 to all the digits and append byte 0x0 at the end. Result will have length n-m+w+1

	hash_type hashType(){return type;}

	string toString(ulint W);//converts fingerprint of length w to string

	SetZIterator * getSetZIterator(){return new SetZIterator(this);}

 	uint base,m,w,r,log_base;

	hash_type type;

	void saveToFile(FILE *fp);
	void loadFromFile(FILE *fp);

protected:

 	uint digitAt(ulint W, uint i);//i-th digit from right
 	void buildZSet();
 	void quickSort(ulint *arr, uint n);
 	bool searchInZ(ulint x, uint err);
 	static bool binarySearch(ulint *arr, uint n, ulint x);

	const static uint radius = 2;//by default, Z set is built with at maximum radius 3 (in practice, at most 2 errors are introduced in the seeds)

	uint *code;//digit associated with each char in the text. Needed to compute the numeric hash value
	bool *random_char;//assign a random digit to this char?

	ulint **Z;//set Z
	uint *Z_size;//size of Z(i)

	// ---------- init the type of hash function

	void init_DEFAULT();
	void init_DNA_SEARCH();
	void init_BS_SEARCH();

};
}

#endif /* HASHFUNCTION_H_ */
