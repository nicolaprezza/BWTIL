/*
 * DummyDynamicBitvector.h
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#ifndef DUMMYDYNAMICBITVECTOR_H_
#define DUMMYDYNAMICBITVECTOR_H_

#include "../common/common.h"

namespace bwtil {

class DummyDynamicBitvector {
public:

	DummyDynamicBitvector(){};
	DummyDynamicBitvector(ulint n);//allocate memory for bitvector of max length n
	virtual ~DummyDynamicBitvector();

	bool access(ulint i);
	ulint size(){return current_size;};//current size
	void insert(bool x, ulint i);
	ulint rank(bool x, ulint i);

	void print();

	uint height();//height of the tree

	ulint maxSize(){return n;};

private:

	static const uint W = 512;//word length

	ulint rank1(ulint i);

	ulint current_size;
	ulint n;
	vector<bool> * bitvector;
	vector<bool> * waste;

};

} /* namespace compressed_bwt_construction */
#endif /* DUMMYDYNAMICBITVECTOR_H_ */
