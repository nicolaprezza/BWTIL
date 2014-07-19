/*
 * DummyDynamicBitvector.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#include "DummyDynamicBitvector.h"

namespace bwtil {

DummyDynamicBitvector::DummyDynamicBitvector(ulint n) {

	this->n = n;
	bitvector = new vector<bool>(n);
	waste = new vector<bool>((n*2)/10);
	current_size = 0;

}

DummyDynamicBitvector::~DummyDynamicBitvector() {}

bool DummyDynamicBitvector::access(ulint i){

	if(i>=current_size)
		cout << "WARNING: access in position " << i << " >= current size of the bitvector (" <<  current_size << ")\n";

	return bitvector->at(i);

}

void DummyDynamicBitvector::insert(bool x, ulint i){

	if(i>current_size)
		cout << "WARNING (DummyDynamicBitvector): insert in position " << i << " > current size of the bitvector (" <<  current_size << ")\n";

	for(ulint j = current_size;j>i;j--)
		bitvector->at(j) = bitvector->at(j-1);

	bitvector->at(i) = x;

	current_size++;

	if(current_size>n)
		cout << "WARNING (DummyDynamicBitvector): maximum size exceeded\n";


}

void DummyDynamicBitvector::print(){

	for(ulint i=0;i<size();i++)
		cout << bitvector->at(i);

	cout << endl;

}

ulint DummyDynamicBitvector::rank(bool x, ulint i){

	if(x==1)
		return rank1(i);

	return i-rank1(i);

}

ulint DummyDynamicBitvector::rank1(ulint i){

	ulint r=0;

	for(ulint j=0;j<i;j++)
		r += bitvector->at(j);

	return r;

}


} /* namespace compressed_bwt_construction */
