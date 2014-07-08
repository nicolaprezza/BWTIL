/*
 * DynamicString.h
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#ifndef DYNAMICSTRING_H_
#define DYNAMICSTRING_H_

#include "../common/common.h"
#include "HuffmanTree.h"
#include "DummyDynamicBitvector.h"

#include <sstream>

namespace bwtil {

class DynamicString {

public:

	DynamicString();

	virtual ~DynamicString();

	virtual symbol access(ulint i){return 0;};
	ulint size(){return current_size;};//current size
	virtual void insert(symbol x, ulint i){

		cout << "ERROR (DynamicString): insert in empty string.\n";
		exit(0);

	}
	virtual ulint rank(symbol x, ulint i){return 0;};

	ulint maxLength(){return n;}

	static DynamicString * getDynamicString(vector<ulint> * freq);

	string toString();

	double entropy(){return H0;}

protected:

	ulint current_size;
	symbol sigma;
	symbol sigma_0;

	double H0;//0-th order entropy reached by the Huffman compressor

	ulint n;//max size of the string

};

} /* namespace compressed_bwt_construction */
#endif /* DYNAMICSTRING_H_ */
