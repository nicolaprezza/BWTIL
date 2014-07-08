//============================================================================
// Name        : IndexedBWT.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : BWT with succinct rank structures and sampled SA pointers

/*   This class stores the BWT as a wavelet tree + structures to retrieve original text addresses from BWT addresses.
 *   implements rank functions on the BWT.
 *
 *   ASSUMPTION: the array 'BWT' must be a BWT (of length n) of some text, with 0x0 byte as terminator character AND no other 0x0 bytes
 *   The class will perform a re-mapping of the bwt, subtracting 1 to each character (except the terminator character) to keep the alphabet size at a minimum.
 *
 */
//============================================================================

#ifndef INDEXEDBWT_H_
#define INDEXEDBWT_H_

#include "WaveletTree.h"
#include "StaticBitVector.h"
#include "WordVector.h"

namespace bwtil {

class IndexedBWT {
public:

	IndexedBWT(){};
	IndexedBWT(unsigned char * BWT, ulint n, ulint offrate, bool verbose = false);

	virtual ~IndexedBWT();

	ulint rank(unsigned char c, ulint i);//number of characters 'c' before position i excluded

	unsigned char at(ulint i);
	unsigned char charAt_remapped(ulint i);

	ulint length(){return n;}

	ulint LF(ulint i);//LF mapping from last column to first

	ulint convertToTextCoordinate(ulint i);//i=address on BWT (F column). returns corresponding address on text

	void test();

	ulint size();//returns size of the structure in bits

	uint TERMINATOR;//terminator character

	//backward search: search the suffix of length 'length' of the word W, where each character is formed by 'log_sigma' bits
	//returns interval <lower_included, upper_excluded> on the BWT
	pair<ulint, ulint> BS(ulint W, uint length,pair<ulint, ulint> interval = pair<ulint, ulint>(0,0));

	vector<ulint> convertToTextCoordinates(pair<ulint, ulint> interval);

	void freeMemory();

	void saveToFile(FILE *fp);
	void loadFromFile(FILE *fp);

private:



	//returns i-th digit of log_sigma bits from left in the suffix of length 'length' of the word W
	uint digitAt(ulint W, uint i);

	void markPositions();//mark 1 every offrate positions of the text on the bwt (vector marked_positions)
	void sampleSA();//sample SA pointers (1 every offrate positions on text). Vector marked_positions must be already computed!

	uint sigma;//alphabet size (excluded terminator character)
	uint log_sigma;//number of bits of each character

	ulint n;//BWT length (included terminator character)
	ulint terminator_position;//position of the terminator character in the BWT
	ulint offrate;//distance on the text between 2 marked positions

	ulint number_of_SA_pointers;

	uint w;//size of a pointer = log2 n

	WaveletTree * bwt_wt;//BWT stored as a wavelet tree
	StaticBitVector * marked_positions;//marks positions on the BWT having a text-pointer
	WordVector * text_pointers;//stores sampled text pointers (SA pointers)

	ulint * FIRST;//first column in the matrix of the ordered suffixes. FIRST[c]=position of the first occurrence of c in the 1st column

};

} /* namespace data_structures */
#endif /* INDEXEDBWT_H_ */
