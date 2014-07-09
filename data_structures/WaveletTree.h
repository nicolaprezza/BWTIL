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
#include "WordVector.h"

namespace bwtil {

class WaveletTree {
public:

	WaveletTree(){};
	WaveletTree(unsigned char * text, ulint n, bool verbose=false);

	ulint numberOfNodes(){return number_of_nodes;};
	ulint height(){return log_sigma;};

	ulint rank(unsigned char c, ulint i);//number of characters 'c' before position i excluded
	unsigned char charAt(ulint i);

	ulint length(){return n;}

	virtual ~WaveletTree();

	uint alphabetSize(){return sigma;}
	uint bitsPerSymbol(){return log_sigma;}

	ulint size();//returns size of the structure in bits

	void freeMemory();

	void saveToFile(FILE *fp);
	void loadFromFile(FILE *fp);

private:

	void buildRecursive(ulint node, WordVector * text_wv,bool verbose);
	uint bitInWord(ulint W, uint i);//i-th bit from left in the word w (of log_sigma bits)
	ulint recursiveRank(unsigned char c, ulint i, ulint node, uint level);

	ulint root(){return 0;}
	ulint child0(ulint node){return 2*node+1;}
	ulint child1(ulint node){return 2*node+2;}


	ulint n;//text length

	StaticBitVector ** nodes;//the tree: a vector of bitvectors
	uint sigma;//alphabet size
	uint log_sigma;//number of bits for each symbol
	ulint number_of_nodes;

	uint * remapping;
	uint * inverse_remapping;

};

} /* namespace data_structures */
#endif /* WAVELETTREE_H_ */
