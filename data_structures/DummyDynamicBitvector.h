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
	DummyDynamicBitvector(ulint n, size_t node_size=0);//allocate memory for bitvector of max length n. node size not used.
	virtual ~DummyDynamicBitvector();

	bool access(ulint i);
	ulint size(){return current_size;};//current size

	void insert(ulint i, bool x);

	ulint rank(ulint i, bool x = true);

	void print();

	uint height();//height of the tree

	ulint maxSize(){return n;};

	struct info_t {
	            const size_t capacity;
	            const size_t size;
	            const size_t height;
	            const size_t node_width;
	            const size_t counter_width;
	            const size_t pointer_width;
	            const size_t degree;
	            const size_t buffer;
	            const size_t nodes;
	            const size_t leaves;
	        };

	info_t info() const {

		return {

			n,
			current_size,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0

		};

	};

private:

	static const uint W = 512;//word length

	ulint rank1(ulint i);

	ulint current_size;
	ulint n;
	vector<bool> bitvector;
	vector<bool> waste;

};

} /* namespace compressed_bwt_construction */
#endif /* DUMMYDYNAMICBITVECTOR_H_ */
