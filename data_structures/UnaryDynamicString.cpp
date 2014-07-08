/*
 * UnaryDynamicString.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#include "UnaryDynamicString.h"

namespace bwtil {

UnaryDynamicString::UnaryDynamicString(ulint n, vector<ulint> * freq){

	//ASSUMPTION: only one entry in freq is > 0

	for(uint i=0;i<freq->size();i++)//search the unique char with freq>0
		if(freq->at(i)>0)
			s=i;

	current_size=0;

	sigma_0=1;
	sigma=freq->size();

	this->n=n;

	H0 = (double)64/(double)n;

}

symbol UnaryDynamicString::access(ulint i){

#ifdef DEBUG
	if(i>=current_size){

		cout << "ERROR (UnaryDynamicString): trying to access position outside current string : " << i << ">=" << current_size << endl;
		exit(0);

	}
#endif

	return s;
}

void UnaryDynamicString::insert(symbol x, ulint i){

#ifdef DEBUG
	if(x!=s){
		cout << "ERROR (UnaryDynamicString): trying to insert a symbol not in the alphabet\n";
		exit(0);
	}
#endif

	current_size++;

}

ulint UnaryDynamicString::rank(symbol x, ulint i){

#ifdef DEBUG
	if(i>current_size){

		cout << "ERROR (UnaryDynamicString): trying to compute rank in position outside current string : " << i << ">" << current_size << endl;
		exit(0);

	}
#endif

	//ASSUMPTION:i<=current_size
	return i;


}

} /* namespace compressed_bwt_construction */
