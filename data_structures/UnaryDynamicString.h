/*
 * UnaryDynamicString.h
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#ifndef UNARYDYNAMICSTRING_H_
#define UNARYDYNAMICSTRING_H_

#include "DynamicString.h"

namespace bwtil {

class UnaryDynamicString  : public DynamicString {

public:

	UnaryDynamicString(){};

	UnaryDynamicString(ulint n, vector<ulint> * freq);//maximum size of the string and absolute frequencies of the characters

	symbol access(ulint i);
	void insert(symbol x, ulint i);
	ulint rank(symbol x, ulint i);

	symbol s;//only symbol allowed

};

} /* namespace compressed_bwt_construction */
#endif /* UNARYDYNAMICSTRING_H_ */
