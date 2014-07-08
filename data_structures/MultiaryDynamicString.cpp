/*
 * MultiaryDynamicString.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#include "MultiaryDynamicString.h"

namespace bwtil {

MultiaryDynamicString::MultiaryDynamicString(ulint n, vector<ulint> * freq){//maximum size of the string and absolute frequencies of the characters

#ifdef DEBUG

	this->freq = new vector<ulint>(freq->size());

	for(ulint i=0;i<freq->size();i++)
	this->freq->at(i) = freq->at(i);

#endif

	HuffmanTree ht = HuffmanTree(freq);
	codes = ht.getCodes();

	H0 = ht.entropy();

	this->n = n;

#ifdef DEBUG
	current_freqs = new vector<ulint>(freq->size());
	for(uint i=0;i<freq->size();i++)
		current_freqs->at(i)=0;
#endif

	current_size = 0;

	sigma = freq->size();

	sigma_0=0;

	for(uint i=0;i<freq->size();i++)
		if(freq->at(i)>0)
			sigma_0++;

	number_of_internal_nodes = sigma_0-1;

	wavelet_tree = new DummyDynamicBitvector[number_of_internal_nodes];
	child0 = new uint16_t[number_of_internal_nodes];
	child1 = new uint16_t[number_of_internal_nodes];

	vector<symbol> alphabet;
	for(symbol i=0;i<freq->size();i++)
		if(freq->at(i)>0)
			alphabet.push_back(i);

	uint next_free_node = 1;
	buildTree(freq,alphabet,0,0,&next_free_node);

}

void MultiaryDynamicString::buildTree(vector<ulint> * freq,vector<symbol> alphabet,uint pos,uint this_node, uint * next_free_node){

	vector<symbol> alphabet0;
	vector<symbol> alphabet1;

	ulint size = 0;//size of the current bitvector

	for(uint i=0;i<alphabet.size();i++){

		size += freq->at(alphabet.at(i));

	}

	wavelet_tree[this_node] = DummyDynamicBitvector(size);

	for(uint i=0;i<alphabet.size();i++){

		if(codes.at(alphabet.at(i)).at(pos)==0){//left (bit 0)

			if(codes.at(alphabet.at(i)).size()-1==pos){//leaf on left: save character

				child0[this_node] = sigma+alphabet.at(i);

			}else{

				if(alphabet0.size()==0){//if this is the first symbol seen with bit 0, allocate new tree node
					child0[this_node] = *next_free_node;
					*next_free_node += 1;
				}

				alphabet0.push_back(alphabet.at(i));

			}

		}else{//right (bit 1)

			if(codes.at(alphabet.at(i)).size()-1==pos){//leaf on right: save character

				child1[this_node] = sigma+alphabet.at(i);

			}else{

				if(alphabet1.size()==0){//if this is the first symbol seen with bit 1, allocate new tree node
					child1[this_node] = *next_free_node;
					*next_free_node += 1;
				}

				alphabet1.push_back(alphabet.at(i));

			}

		}

	}

	if(alphabet0.size()>0)
		buildTree(freq,alphabet0,pos+1,child0[this_node],next_free_node);

	if(alphabet1.size()>0)
		buildTree(freq,alphabet1,pos+1,child1[this_node],next_free_node);


}

symbol MultiaryDynamicString::access(ulint i){

#ifdef DEBUG
	if(i>=current_size){

		cout << "ERROR (MultiaryDynamicString): trying to access position outside current string : " << i << ">=" << current_size << endl;
		exit(0);

	}
#endif

	return access(0,i);

}

symbol MultiaryDynamicString::access(uint node, ulint i){

	bool bit = wavelet_tree[node].access(i);

	uint next_node = (bit==0?child0[node]:child1[node]);

	if(next_node>=sigma)//next node is leaf:return symbol
		return next_node-sigma;

	//else: next_node is a valid address in wavelet_tree

	ulint next_i = wavelet_tree[node].rank(bit,i);

	return access(next_node, next_i);

}

void MultiaryDynamicString::insert(symbol x, ulint i){

#ifdef DEBUG
	if(i>current_size){

		cout << "ERROR (MultiaryDynamicString): trying to insert in position outside current string : " << i << ">" << current_size << endl;
		exit(0);

	}

	if(current_freqs->at(x)>=freq->at(x)){

		cout << "ERROR (MultiaryDynamicString): too many symbols " << (uint)x << " inserted!" << endl;
		exit(0);

	}

	current_freqs->at(x) += 1;

#endif



	vector<bool> code = codes.at(x);//bitvector to be inserted in the wavelet tree

	insert(&code,0,0,i);

	current_size++;

}

void MultiaryDynamicString::insert(vector<bool> * code, uint node, uint pos, ulint i){

	bool bit = code->at(pos);

	wavelet_tree[node].insert( bit, i );//insert the bit in the wavelet tree node

	if(pos+1<code->size()){

		uint next_node = (bit==0?child0[node]:child1[node]);//find next node
		ulint next_i = wavelet_tree[node].rank(bit,i);
		insert(code, next_node, pos+1, next_i);

	}

}

ulint MultiaryDynamicString::rank(symbol x, ulint i){

#ifdef DEBUG
	if(i>current_size){

		cout << "ERROR (MultiaryDynamicString): trying to compute rank in position outside current string : " << i << ">" << current_size << endl;
		exit(0);

	}
#endif

	vector<bool> code = codes.at(x);//bitvector to be searched in the wavelet tree

	return rank(&code, 0, 0, i);

}

ulint MultiaryDynamicString::rank(vector<bool> * code, uint node, uint pos, ulint i){

	bool bit = code->at(pos);
	ulint bit_rank = wavelet_tree[node].rank(bit,i);

	if(pos+1==code->size())
		return bit_rank;

	uint next_node = (bit==0?child0[node]:child1[node]);//find next node

	return rank(code, next_node, pos+1, bit_rank);

}


} /* namespace compressed_bwt_construction */
