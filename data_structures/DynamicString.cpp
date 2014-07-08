/*
 * DynamicString.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#include "DynamicString.h"

#include "MultiaryDynamicString.h"
#include "UnaryDynamicString.h"

namespace bwtil {

DynamicString::DynamicString(){current_size=0;}

DynamicString::~DynamicString(){};

DynamicString * DynamicString::getDynamicString(vector<ulint> * freq){

	ulint n=0;

	for(uint i=0;i<freq->size();i++)
		n+=freq->at(i);

	if(n==0)
		return new DynamicString();//empty

#ifdef DEBUG
	if(freq->size()>255){
		cout << "ERROR (DynamicString): Maximum size of the alphabet is 255. (input alphabet size is " << freq->size() << ")\n";
		exit(0);
	}
#endif

	symbol _sigma_0=0;

	for(uint i=0;i<freq->size();i++)
		if(freq->at(i)>0)
			_sigma_0++;

#ifdef DEBUG
	if(_sigma_0==0){
		cout << "ERROR (DynamicString): trying to build dynamic string on a null alphabet\n";
		exit(0);
	}
#endif

	//if the string is on a unary alphabet, it is sufficient to count the number of symbols inserted
	if(_sigma_0==1)
		return new UnaryDynamicString(n,freq);

	//here we know that alphabet has at least size 2

	return new MultiaryDynamicString(n,freq);

}

string DynamicString::toString(){

	stringstream ss;

	for(ulint i=0;i<size();i++)
		ss << (uint)access(i);

	return ss.str();

}


} /* namespace compressed_bwt_construction */
