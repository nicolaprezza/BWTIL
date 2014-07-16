/*
 * DynamicString.h
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#ifndef DYNAMICSTRING_H_
#define DYNAMICSTRING_H_

#include "../common/common.h"
#include "HuffmanTree.h"
#include "DummyDynamicBitvector.h"

#include <sstream>

namespace bwtil {

class DynamicString {

public:

	DynamicString();

	DynamicString(vector<ulint> * freq);//maximum size of the string and absolute frequencies of the characters

	symbol access(ulint i);
	void insert(symbol x, ulint i);
	ulint rank(symbol x, ulint i);

	ulint size(){return current_size;};//current size

	ulint maxLength(){return n;}

	string toString();

	double entropy(){return H0;}

private:

	void buildTree(vector<ulint> * freq,vector<symbol> alphabet,uint pos, uint this_node, uint * next_free_node);

	void insert(vector<bool> * code, uint node, uint pos, ulint i);
	symbol access(uint node, ulint i);
	ulint rank(vector<bool> * code, uint node, uint pos, ulint i);

	vector<vector<bool> > codes;//Huffman codes

#ifdef DEBUG

	vector<ulint> * freq;//debug only: absolute frequencies of the symbols
	vector<ulint> * current_freqs;//debug only: number of symbols inserted

#endif

	DummyDynamicBitvector * wavelet_tree;//internal nodes of the wavelet tree (number_of_internal_nodes in total)
	//tree topology
	symbol number_of_internal_nodes;
	uint16_t * child0;//for each node, pointer to left child in wavelet_tree (if any; otherwise sigma+s, where s is the symbol associated to the leaf)
	uint16_t * child1;//for each node, pointer to right child in wavelet_tree (if any; otherwise sigma+s, where s is the symbol associated to the leaf)

	symbol s;//unique symbol

	bool unary_string;//alphabet has size 1
	ulint current_size;
	symbol sigma;
	symbol sigma_0;

	double H0;//0-th order entropy reached by the Huffman compressor

	ulint n;//max size of the string

};

} /* namespace compressed_bwt_construction */
#endif /* DYNAMICSTRING_H_ */
