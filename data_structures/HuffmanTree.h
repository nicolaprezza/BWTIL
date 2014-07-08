/*
 * HuffmanTree.h
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#ifndef HUFFMANTREE_H_
#define HUFFMANTREE_H_

#include "../common/common.h"

namespace bwtil {

class HuffmanTree {
public:

	HuffmanTree(){};

	HuffmanTree(vector<ulint> * freq);
	virtual ~HuffmanTree();

	ulint numberOfOccurrencies(symbol s){return frequencies->at(s);};//number of occurrencies of the symbol s

	vector<vector<bool> > getCodes(){return codes;}

	vector<bool> code(symbol s){return codes.at(s);};//from symbol -> to its binary Huffman code (compression)
	//symbol decode(vector<bool> c);//from binary Huffman code -> to symbol (decompression)

	double entropy();//get entropy (bits per symbol)

	void debug();

private:

	class Node{

	public:

		Node(symbol label, ulint freq){
			this->freq=freq;
			this->label=label;
			leaf=true;
		}

		Node(symbol nr, Node l, Node r){

			node_number=nr;
			freq = l.freq+r.freq;
			leaf=false;
			leaf_right = r.leaf;
			leaf_left = l.leaf;

			if(leaf_left)
				left = l.label;
			else
				left = l.node_number;

			if(leaf_right)
				right = r.label;
			else
				right = r.node_number;

		}

		bool operator<(const Node & n) const { return freq<n.freq; };

		symbol node_number;
		ulint freq;
		symbol label;//label if leaf
		bool leaf;//this node is leaf
		bool leaf_left;
		bool leaf_right;

		symbol left;//pointers to children
		symbol right;

	};

	void storeTree(Node n,vector<Node> nodes_vec);
	void storeTree(vector<bool> code, Node n,vector<Node> nodes_vec);

	//symbol decode(vector<bool> c, uint pos, symbol node);

	uint8 sigma_0;//nr of symbols
	uint8 sigma;//nr of symbols with frequency > 0

	//the Huffman tree:
	//symbol root_node;
	//symbol * left;//sigma-1 left pointers
	//symbol * right;//sigma-1 right pointers
	//vector<bool> left_leafs;//for each internal node i, memorizes if the symbol in left is pointer to internal nodes (0) or leaf label (1)
	//vector<bool> right_leafs;//for each internal node i, memorizes if the symbol in left is pointer to internal nodes (0) or leaf label (1)

	vector<ulint> * frequencies;//table of sigma_0 entries. Symbol -> number of occurrencies.

	vector<vector<bool> > codes;

};

} /* namespace compressed_bwt_construction */
#endif /* HUFFMANTREE_H_ */
