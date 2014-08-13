/*
 * DynamicString.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#include "DynamicString.h"

namespace bwtil {

DynamicString::DynamicString(){n=0;current_size=0;unary_string=true;sigma=0;sigma_0=0;H0=0;}

DynamicString::DynamicString(vector<ulint> * freq){//absolute frequencies of the characters

	n=0;

	for(uint i=0;i<freq->size();i++)
		n+=freq->at(i);

	if(n==0){
		current_size=0;
		unary_string=true;
		sigma=0;
		sigma_0=0;
		H0=0;
		return;
	}

#ifdef DEBUG
	if(freq->size()>255){
		cout << "ERROR (DynamicString): Maximum size of the alphabet is 255. (input alphabet size is " << freq->size() << ")\n";
		exit(0);
	}

	this->freq = new vector<ulint>(freq->size());

	for(ulint i=0;i<freq->size();i++)
		this->freq->at(i) = freq->at(i);

#endif

	sigma = freq->size();
	sigma_0=0;

	for(uint i=0;i<freq->size();i++)
		if(freq->at(i)>0)
			sigma_0++;

#ifdef DEBUG
	if(sigma_0==0){
		cout << "ERROR (DynamicString): trying to build dynamic string on a null alphabet\n";
		exit(0);
	}

	current_freqs = new vector<ulint>(freq->size());
	for(uint i=0;i<freq->size();i++)
		current_freqs->at(i)=0;
#endif

	current_size = 0;

	if(sigma_0==1){

		for(uint i=0;i<freq->size();i++)//search the unique char with freq>0
			if(freq->at(i)>0)
				s=i;

		unary_string = true;
		H0 = (uint)log2(n);
		return;

	}

	//Alphabet size is > 1

	unary_string = false;

	HuffmanTree ht = HuffmanTree(freq);
	codes = ht.getCodes();

	H0 = ht.entropy();

	number_of_internal_nodes = sigma_0-1;

	wavelet_tree.resize(number_of_internal_nodes);

	child0 = new uint16_t[number_of_internal_nodes];
	child1 = new uint16_t[number_of_internal_nodes];

	vector<symbol> alphabet;
	for(symbol i=0;i<freq->size();i++)
		if(freq->at(i)>0)
			alphabet.push_back(i);

	uint next_free_node = 1;
	buildTree(freq,alphabet,0,0,&next_free_node);

}

void DynamicString::buildTree(vector<ulint> * freq,vector<symbol> alphabet,uint pos,uint this_node, uint * next_free_node){

	vector<symbol> alphabet0;
	vector<symbol> alphabet1;

	ulint size = 0;//size of the current bitvector

	for(uint i=0;i<alphabet.size();i++){

		size += freq->at(alphabet.at(i));

	}

	wavelet_tree[this_node] = bitv(size,node_bitsize);

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

symbol DynamicString::access(ulint i){

	if(n==0)
		return 0;

#ifdef DEBUG
	if(i>=current_size){

		cout << "ERROR (DynamicString): trying to access position outside current string : " << i << ">=" << current_size << endl;
		exit(0);

	}
#endif

	if(unary_string){

		return s;

	}

	return access(0,i);

}

symbol DynamicString::access(uint node, ulint i){

	bool bit = wavelet_tree[node].access(i);

	uint next_node = (bit==0?child0[node]:child1[node]);

	if(next_node>=sigma)//next node is leaf:return symbol
		return next_node-sigma;

	//else: next_node is a valid address in wavelet_tree

	ulint next_i = wavelet_tree[node].rank(i,bit);

	return access(next_node, next_i);

}

void DynamicString::insert(symbol x, ulint i){

	if(n==0)
		return;

#ifdef DEBUG
	if(i>current_size){

		cout << "ERROR (DynamicString): trying to insert in position outside current string : " << i << ">" << current_size << endl;
		exit(0);

	}

	if(current_freqs->at(x)>=freq->at(x)){

		cout << "ERROR (DynamicString): too many symbols " << (uint)x << " inserted!" << endl;
		exit(0);

	}

	current_freqs->at(x) += 1;

#endif


	if(not unary_string){

		vector<bool> code = codes.at(x);//bitvector to be inserted in the wavelet tree
		insert(&code,0,0,i);

	}

	current_size++;

}

void DynamicString::insert(vector<bool> * code, uint node, uint pos, ulint i){

	bool bit = code->at(pos);

	//wavelet_tree[node].insert( bit, i );//insert the bit in the wavelet tree node

	/*cout << "node = " << (uint)node << "/" <<  (uint)number_of_internal_nodes << endl;
	cout << "bitv capacity: " << wavelet_tree[node].info().capacity << endl;
	cout << "bitv size : " << wavelet_tree[node].info().size << endl;
	cout << "i,bit = " << i << ", " << bit << endl;*/

	/*if(wavelet_tree[node].info().capacity==7716){

		cout << "Entering test zone"<<endl;

		bitv bbb(7716,256);
		bbb.insert(0,1);

		cout << "Test works"<<endl;//TODO debugging!!


		bitv aaa = wavelet_tree[node];

		cout << "Assigned"<<endl;

		aaa.insert(0,1);

		cout << "Exit test zone"<<endl;

	}*/

	wavelet_tree[node].insert( i, bit );

	if(pos+1<code->size()){

		uint next_node = (bit==0?child0[node]:child1[node]);//find next node

		//ulint next_i = wavelet_tree[node].rank(bit,i);
		ulint next_i = wavelet_tree[node].rank(i,bit);

		insert(code, next_node, pos+1, next_i);

	}

}

ulint DynamicString::rank(symbol x, ulint i){

	if(n==0)
		return 0;

#ifdef DEBUG
	if(i>current_size){

		cout << "ERROR (DynamicString): trying to compute rank in position outside current string : " << i << ">" << current_size << endl;
		exit(0);

	}
#endif

	if(unary_string)
		return i;

	vector<bool> code = codes.at(x);//bitvector to be searched in the wavelet tree
	return rank(&code, 0, 0, i);

}

ulint DynamicString::rank(vector<bool> * code, uint node, uint pos, ulint i){

	bool bit = code->at(pos);
	//ulint bit_rank = wavelet_tree[node].rank(bit,i);
	ulint bit_rank = wavelet_tree[node].rank(i,bit);

	if(pos+1==code->size())
		return bit_rank;

	uint next_node = (bit==0?child0[node]:child1[node]);//find next node

	return rank(code, next_node, pos+1, bit_rank);

}

string DynamicString::toString(){

	stringstream ss;

	for(ulint i=0;i<size();i++)
		ss << (uint)access(i);

	return ss.str();

}

ulint DynamicString::numberOfBits(){//sum of the lengths of the bitvectors

	if(unary_string)
		return n;

	ulint tot=0;

	for(uint i=0;i<number_of_internal_nodes;i++)
		//tot += wavelet_tree[i].maxSize();
		tot += wavelet_tree[i].info().capacity;

	return tot;

}

ulint  DynamicString::sumOfHeights(){//sum of the heights of all bitvectors' B-trees (each multiplied by the length of the bitvector)

	if(unary_string)
		return n;

	ulint tot=0;

	for(uint i=0;i<number_of_internal_nodes;i++)
		//tot += wavelet_tree[i].maxSize()*wavelet_tree[i].height();
		tot += wavelet_tree[i].info().capacity * wavelet_tree[i].info().height;

	return tot;

}

} /* namespace compressed_bwt_construction */
