/*
 * ContextAutomata.cpp
 *
 *  Created on: Jul 14, 2014
 *      Author: nicola
 */

#include "ContextAutomata.h"

namespace bwtil {

ContextAutomata::ContextAutomata(uint sigma,uint k){

	this->sigma=sigma;
	this->k=k;

	sigma_pow_k_minus_one = 1;
	for(uint i=0;i<k-1;i++)
		sigma_pow_k_minus_one *= sigma;

	null_ptr = ~((uint)0);

	edges.push_back(vector<uint>(sigma,null_ptr));



}


} /* namespace bwtil */
