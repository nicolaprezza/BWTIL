/*
 * succinctFMIndex.h
 *
 *  Created on: Jul 23, 2014
 *      Author: nicola
 *
 *  Description: an uncompressed (yet succinct) wavelet-tree based FM index.
 *
 */

#ifndef SUCCINCTFMINDEX_H_
#define SUCCINCTFMINDEX_H_

#include "../common/common.h"
#include "IndexedBWT.h"
#include "../extern/sais.h"
#include "FileReader.h"

namespace bwtil {

class succinctFMIndex {

public:

	succinctFMIndex(){};

	succinctFMIndex(string path, bool verbose = false);

	succinctFMIndex(unsigned char * text, ulint n, bool verbose = false);

	static unsigned char * computeBWT(unsigned char* text, ulint length);

	//apply backward search and return occurrencies of P in the text
	vector<ulint> getOccurrencies(string P);

	ulint size();//returns size of the structure in bits

	void saveToFile(string path);
	static succinctFMIndex loadFromFile(string path);

	void saveToFile(FILE *fp);
	void loadFromFile(FILE *fp);

	ulint textLength(){return n;};

	void load(string path);

private:

	void build(unsigned char * text, ulint n, bool verbose);

	IndexedBWT idxBWT;
	ulint n;//text length (excluded terminator character 0x0)

	uint sigma;//alphabet size
	uint log_sigma;//log2(sigma)

	ulint offrate;

};

} /* namespace bwtil */
#endif /* SUCCINCTFMINDEX_H_ */
