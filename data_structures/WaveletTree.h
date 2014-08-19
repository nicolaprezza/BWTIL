//============================================================================
// Name        : WaveletTree.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : 	This class implements a binary balanced wavelet tree on the input text.
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

	WaveletTree(string text, bool verbose=false){

		if (verbose) cout << "  Building Wavelet tree"<<endl;

		this->n = text.length();
		sigma = 0;

		for(ulint i=0;i<n;i++)
			if((uchar)text[i]>sigma)
				sigma = (uchar)text[i];

		sigma++;

		log_sigma = ceil(log2(sigma));

		auto text_wv = new packed_view_t(log_sigma,n);
		for(ulint i = 0;i<n;i++)
			(*text_wv)[i] = (uchar)text[i];

		number_of_nodes = ((ulint)1<<log_sigma)-1;

		if (verbose) cout << "   Number of nodes = "<< number_of_nodes << endl;

		nodes = vector<StaticBitVector>(number_of_nodes);

		if (verbose) cout << "   Recursively building nodes" << endl;
		buildRecursive(root(),text_wv,verbose);

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

	void buildRecursive(ulint node, packed_view_t * text_wv,bool verbose){

		if (verbose) cout << "    Building node number " << node << endl;
		if (verbose) cout << "     Node length = " << text_wv->size() << " bits" << endl;

		nodes[node] = StaticBitVector(text_wv->size());

		uint width = text_wv->width();

		for(ulint i=0;i<text_wv->size();i++)
			nodes[node].setBit(i,((ulint)(*text_wv)[i])>>(width-1));//leftmost bit of the i-th word in text_wv

		if (verbose) cout << "     Computing rank structure ... " << flush;
		nodes[node].computeRanks();
		if (verbose) cout << "Done."<<endl;

		if(width>1){//if there are children

			//children
			auto text_child0 = new packed_view_t(width-1,nodes[node].numberOf0());
			auto text_child1 = new packed_view_t(width-1,nodes[node].numberOf1());

			ulint j0=0;
			ulint j1=0;

			ulint MASK = (((ulint)1)<<(width-1))-1;

			for(ulint i=0;i<text_wv->size();i++){//for each symbol in text_wv

				if(nodes[node].bitAt(i)==0){//if first bit is 0
					(*text_child0)[j0] = ((ulint)(*text_wv)[i] & MASK);//we take the word in position i and we remove the leftmost bit
					j0++;
				}else{
					(*text_child1)[j1] = ((ulint)(*text_wv)[i] & MASK);
					j1++;
				}

			}

			//delete text_wv;//TODO to debug! if removed, the output is not correct.

			//call recursively

			buildRecursive(child0(node),text_child0,verbose);
			buildRecursive(child1(node),text_child1,verbose);

		}//else
			//delete text_wv;//delete leaf

	}

	inline uint bitInWord(ulint W, uint i){
		return (W>>(log_sigma-i-1))&1;
	}

	ulint recursiveRank(uchar c, ulint i, ulint node, uint level){//number of characters 'c' before position i excluded

		if(nodes[node].length()==0)//empty node
			return 0;

		uint bit = bitInWord(c,level);
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
