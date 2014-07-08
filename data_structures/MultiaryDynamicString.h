/*
 * MultiaryDynamicString.h
 *
 *  Created on: Jun 19, 2014
 *      Author: nicola
 */

#ifndef MULTIARYDYNAMICSTRING_H_
#define MULTIARYDYNAMICSTRING_H_

#include "../common/common.h"
#include "DynamicString.h"

namespace bwtil {

class MultiaryDynamicString : public DynamicString {

public:

	MultiaryDynamicString(ulint n, vector<ulint> * freq);//maximum size of the string and absolute frequencies of the characters

	symbol access(ulint i);
	void insert(symbol x, ulint i);
	ulint rank(symbol x, ulint i);

private:

	void buildTree(vector<ulint> * freq,vector<symbol> alphabet,uint pos, uint this_node, uint * next_free_node);

	void insert(vector<bool> * code, uint node, uint pos, ulint i);
	symbol access(uint node, ulint i);
	ulint rank(vector<bool> * code, uint node, uint pos, ulint i);

	vector<vector<bool> > codes;//Huffman codes

#ifdef DEBUG
	vector<ulint> * freq;//absolute frequencies of the symbols
#endif

	DummyDynamicBitvector * wavelet_tree;//internal nodes of the wavelet tree (number_of_internal_nodes in total)
	//tree topology
	symbol number_of_internal_nodes;
	uint16_t * child0;//for each node, pointer to left child in wavelet_tree (if any; otherwise sigma+s, where s is the symbol associated to the leaf)
	uint16_t * child1;//for each node, pointer to right child in wavelet_tree (if any; otherwise sigma+s, where s is the symbol associated to the leaf)

#ifdef DEBUG
	vector<ulint> * current_freqs;//debug only: number of symbols inserted
#endif

};

} /* namespace compressed_bwt_construction */
#endif /* MULTIARYDYNAMICSTRING_H_ */
