/*
 * Counters.cpp
 *
 *  Created on: Jun 27, 2014
 *      Author: nicola
 */

#include "Counters.h"

namespace bwtil {

Counters::Counters(uint sigma, ulint n){

	if(n==0)//empty counters
		return;

	this->sigma=sigma;

	log2n = ceil(log2(n+1));

	uint w=64;
	d = w/log2n;

	nr_of_words = sigma/d + (sigma%d==0?0:1);//words storing the counters

	words = new ulint[nr_of_words];
	for(uint i=0;i<nr_of_words;i++)//reset all counters
		words[i]=0;

}

void Counters::increment(symbol s){//increment by 1 the counter in s

	uint word_nr = s/d;
	uint offset = s%d;

	words[word_nr] += ((ulint)1)<<((d-offset-1)*log2n);

}

ulint Counters::at(symbol s){

	uint word_nr = s/d;
	uint offset = s%d;
	ulint MASK = (((ulint)1)<<log2n)-1;

	return (words[word_nr]>>((d-offset-1)*log2n)) & MASK;

}

string Counters::toString(){

	stringstream ss;

	for(uint i=0;i<sigma;i++)
		ss << at(i) << " ";

	return ss.str();

}



} /* namespace compressed_bwt_construction */
