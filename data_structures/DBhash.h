//============================================================================
// Name        : DBhash.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : A dB-hash (de Bruijn hash) data structures on general alphabets and texts of length n<2^64.
//============================================================================

#ifndef DBHASH_H_
#define DBHASH_H_

#include "../common/common.h"
#include "HashFunction.h"
#include "IndexedBWT.h"
#include "WordVector.h"
#include "../extern/sais.h"

namespace bwtil {

class DBhash {

public:

	DBhash(){};

	//n = size of the text. bwt is 1 byte longer since it has terminator char 0x0.
	DBhash(unsigned char * text, unsigned char * bwt, ulint n, HashFunction * h, ulint offrate = 4, bool verbose = false);
	DBhash(unsigned char * text, ulint n, HashFunction * h, ulint offrate = 4, bool verbose = false);

	virtual ~DBhash();

	//uses SAIS algorithm to compute BWT
	//max text length = 2^31
	//warning: text must end with 0x0 byte, AND 0x0 byte must not appear elsewhere in the text
	static unsigned char * computeBWT(unsigned char* text, ulint length);

	unsigned char textAt(ulint i);//character in position i of the text

	//returns occurrences in the text of substrings having 'fingerprint' as hash value. uses the auxiliary hash.
	vector<ulint> getOccurrencies(ulint fingerprint);
	//doesn't use auxiliary hash.
	vector<ulint> getOccurrencies_slow(ulint fingerprint);

	//returns occurrences of P (length m) in the text.
	vector<ulint> getOccurrencies(string P, uint max_errors=0);

	//given a pattern, a list of (candidate) occurrencies and a maximum number of errors (Hamming distance), filter out occurrencies at distance > max_errors
	vector<ulint> filterOutBadOccurrencies(string P, vector<ulint> occ, uint max_errors=0);

	ulint size();//returns size of the structure in bits

	void test();

	void freeMemory();

	void saveToFile(string path);
	static DBhash * loadFromFile(string path);

	void saveToFile(FILE *fp);
	void loadFromFile(FILE *fp);

	ulint textLength(){return n;}
	ulint patternLength(){return m;}

	HashFunction * hashFunction(){return h;}

	void load(string path);

protected:

	void initText(unsigned char * text);
	void initAuxHash();

	ulint n,m,w;//text length, pattern length, word length

	uint w_aux;//base^w_aux = number of entries in the auxiliary hash
	ulint auxiliary_hash_size;//number of entries in the auxiliary hash (base^w_aux)

	ulint text_fingerprint_length;

	HashFunction * h;
	ulint offrate;

	IndexedBWT * indexedBWT;
	WordVector * text_wv;//the plain text
	WordVector * auxiliary_hash;

	uint sigma;//alphabet size
	uint log_sigma;//log2(sigma)

	uint * char_to_int;//conversion from a char in the text to an integer in the range {0,...,sigma-1}
	unsigned char * int_to_char;//conversion from int in the range {0,...,sigma-1} to a char

};

} /* namespace data_structures */
#endif /* DBHASH_H_ */
