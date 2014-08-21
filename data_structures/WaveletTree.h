//============================================================================
// Name        : WaveletTree.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : 	This class implements a binary balanced wavelet tree on the input text->
//					Alphabet size is automatically detected, and equals the largest character. Note that this is not optimal if some characters are
//					not present in the text (in this case it is suggested to do a re-mapping of the text before to build the wavelet tree)
//============================================================================


#ifndef WAVELETTREE_H_
#define WAVELETTREE_H_

#include "StaticBitVector.h"

namespace bwtil {

class WaveletTree {
public:

	WaveletTree(){};

	WaveletTree(string * text, bool verbose=false){

		if (verbose) cout << "  Building Wavelet tree"<<endl;

		this->n = text->length();
		sigma = 0;

		for(ulint i=0;i<n;i++)
			if((uchar)text->at(i)>sigma)
				sigma = (uchar)text->at(i);

		sigma++;

		log_sigma = ceil(log2(sigma));

		number_of_nodes = ((ulint)1<<log_sigma)-1;

		if (verbose) cout << "   Number of nodes = "<< number_of_nodes << endl;

		//firstly create a tree of vector<bool>, then switch to StaticBitVectors

		if (verbose) cout << "   filling nodes ... " << endl;

		auto nodes_vec = vector<vector<bool>* >(number_of_nodes);
		for(uint i=0;i<number_of_nodes;i++)
			nodes_vec[i] = new vector<bool>();

		uint node = root();

		int perc=0,last_perc=-1;

		for(ulint i=0;i<n;i++){

			if(perc>last_perc and perc%10==0){

				if (verbose) cout << "   " << perc << "% done." <<endl;
				last_perc=perc;

			}

			for(ulint j=0;j<log_sigma;j++){

				bool bit = bitInChar((uchar)text->at(i),j);

				nodes_vec[node]->push_back(bit);

				node = (bit?child1(node):child0(node));

			}

			node = root();

			perc = (100*i)/n;

		}

		//now switch to StaticBitVectors

		perc=0;
		last_perc=-1;
		if (verbose) cout << "   computing ranks ... " << endl;

		nodes = vector<StaticBitVector>(number_of_nodes);

		for(uint i=0;i<number_of_nodes;i++){

			if(perc>last_perc+10){

				if (verbose) cout << "   " << perc << "% done." <<endl;
				last_perc=perc;

			}

			nodes[i] = StaticBitVector(nodes_vec[i]);
			delete nodes_vec[i];//free node memory

			perc = (100*i)/number_of_nodes;

		}

	}

	inline ulint rank(uchar c, ulint i){//number of characters 'c' before position i excluded

		return recursiveRank(c, i, root(), 0);

	}

	inline uchar charAt(ulint i){

		uchar c=0;
		ulint node = root();
		uint level = 0;
		uint bit=0;

		while(level<height()){

			bit = nodes[node].bitAt(i);
			c = c*2 + bit;

			if(bit==0){
				i = nodes[node].rank0(i);
				node = child0(node);
			}else{
				i = nodes[node].rank1(i);
				node = child1(node);
			}

			level++;

		}

		return c;

	}

	ulint size(){//returns size of the structure in bits

		ulint size = 0;

		for(uint i=0;i<number_of_nodes;i++)
			size += nodes[i].size();

		return size;

	}

	void saveToFile(FILE *fp){

		fwrite(&n, sizeof(ulint), 1, fp);
		fwrite(&sigma, sizeof(uint), 1, fp);
		fwrite(&log_sigma, sizeof(uint), 1, fp);
		fwrite(&number_of_nodes, sizeof(ulint), 1, fp);

		for(ulint i=0;i<number_of_nodes;i++)
			nodes[i].saveToFile(fp);


	}

	void loadFromFile(FILE *fp){

		ulint numBytes;

		numBytes = fread(&n, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&sigma, sizeof(uint), 1, fp);
		check_numBytes();
		numBytes = fread(&log_sigma, sizeof(uint), 1, fp);
		check_numBytes();
		numBytes = fread(&number_of_nodes, sizeof(ulint), 1, fp);
		check_numBytes();

		nodes = vector<StaticBitVector>(number_of_nodes);

		for(ulint i=0;i<number_of_nodes;i++)
			nodes[i].loadFromFile(fp);

	}

	ulint numberOfNodes(){return number_of_nodes;};
	ulint height(){return log_sigma;};

	ulint length(){return n;}

	uint alphabetSize(){return sigma;}
	uint bitsPerSymbol(){return log_sigma;}

private:

	inline uchar bitInChar(uchar W, uint i){
		return (W>>(log_sigma-i-1))&(uchar)1;
	}

	ulint recursiveRank(uchar c, ulint i, ulint node, uint level){//number of characters 'c' before position i excluded

		if(nodes[node].length()==0)//empty node
			return 0;

		uint bit = bitInChar(c,level);
		uint rank;

		if(bit==0)
			rank = nodes[node].rank0(i);
		else
			rank = nodes[node].rank1(i);

		if(level==log_sigma-1)//leaf
			return rank;

		//not a leaf: proceed recursively

		return recursiveRank(c, rank, (bit==0?child0(node):child1(node)), level+1);

	}

	inline ulint root(){return 0;}
	inline ulint child0(ulint node){return 2*node+1;}
	inline ulint child1(ulint node){return 2*node+2;}

	ulint n;//text length

	vector<StaticBitVector> nodes;//the tree: a vector of bitvectors
	uint sigma;//alphabet size
	uint log_sigma;//number of bits for each symbol
	ulint number_of_nodes;

};

} /* namespace data_structures */
#endif /* WAVELETTREE_H_ */
