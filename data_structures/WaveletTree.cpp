//============================================================================
// Name        : WaveletTree.cpp
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : 	This class implements a binary balanced wavelet tree on the input text.
//					Alphabet size is automatically detected, and equals the largest character. Note that this is not optimal if some characters are
//					not present in the text (in this case it is suggested to do a re-mapping of the text before to build the wavelet tree)
//============================================================================


#include "WaveletTree.h"

namespace bwtil {

WaveletTree::WaveletTree(unsigned char * text, ulint n, bool verbose){

	if (verbose) cout << "  Building Wavelet tree"<<endl;

	this->n = n;
	sigma = 0;

	for(ulint i=0;i<n;i++)
		if(text[i]>sigma)
			sigma = text[i];

	sigma++;

	log_sigma = ceil(log2(sigma));

	//store text in WordVector format (which offers useful bit operations)
	WordVector * text_wv = new WordVector(n,log_sigma);
	for(ulint i = 0;i<n;i++)
		text_wv->setWord(i,text[i]);

	number_of_nodes = ((ulint)1<<log_sigma)-1;

	if (verbose) cout << "   Number of nodes = "<< number_of_nodes << endl;

	nodes = new StaticBitVector * [number_of_nodes];

	//ROOT

	if (verbose) cout << "   Recursively building nodes" << endl;
	buildRecursive(root(),text_wv,verbose);

}

void WaveletTree::buildRecursive(ulint node, WordVector * text_wv,bool verbose){

	if (verbose) cout << "    Building node number " << node << endl;
	if (verbose) cout << "     Node length = " << text_wv->length() << " bits" << endl;

	nodes[node] = new StaticBitVector(text_wv->length());

	for(ulint i=0;i<text_wv->length();i++)
		nodes[node]->setBit(i,text_wv->bitAt(i,0));

	if (verbose) cout << "     Computing rank structure ... " << flush;
	nodes[node]->computeRanks();
	if (verbose) cout << "Done."<<endl;

	if(text_wv->wordSize()>1){//if there are children

		cout << "*** 1.1"<<endl;

		cout << "*** wsize = " << text_wv->wordSize()<<endl;
		cout << "*** n0 = " << nodes[node]->numberOf0()<<endl;
		cout << "*** n1 = " << nodes[node]->numberOf1()<<endl;

		//children
		WordVector * text_child0 = new WordVector(nodes[node]->numberOf0(),text_wv->wordSize()-1);
		WordVector * text_child1 = new WordVector(nodes[node]->numberOf1(),text_wv->wordSize()-1);

		cout << "*** 1.2"<<endl;

		ulint j0=0;
		ulint j1=0;

		for(ulint i=0;i<text_wv->length();i++){//for each symbol in text_wv

			if(nodes[node]->bitAt(i)==0){//if first bit is 0
				text_child0->setWord(j0,text_wv->wordAt(i));
				j0++;
			}else{
				text_child1->setWord(j1,text_wv->wordAt(i));
				j1++;
			}

		}

		text_wv->freeMemory();

		//call recursively

		buildRecursive(child0(node),text_child0,verbose);
		buildRecursive(child1(node),text_child1,verbose);

	}

	if(text_wv->wordSize()==1)
		text_wv->freeMemory();

}

uint WaveletTree::bitInWord(ulint W, uint i){
	return (W>>(log_sigma-i-1))&1;
}

ulint WaveletTree::rank(unsigned char c, ulint i){//number of characters 'c' before position i excluded

	return recursiveRank(c, i, root(), 0);

}

ulint WaveletTree::recursiveRank(unsigned char c, ulint i, ulint node, uint level){//number of characters 'c' before position i excluded

	if(nodes[node]->length()==0)//empty node
		return 0;

	uint bit = bitInWord(c,level);
	uint rank;

	if(bit==0)
		rank = nodes[node]->rank0(i);
	else
		rank = nodes[node]->rank1(i);

	if(level==log_sigma-1)//leaf
		return rank;

	//not a leaf: proceed recursively

	return recursiveRank(c, rank, (bit==0?child0(node):child1(node)), level+1);

}

unsigned char WaveletTree::charAt(ulint i){

	unsigned char c=0;
	ulint node = root();
	uint level = 0;
	uint bit=0;

	while(level<height()){

		bit = nodes[node]->bitAt(i);
		c = c*2 + bit;

		if(bit==0){
			i = nodes[node]->rank0(i);
			node = child0(node);
		}else{
			i = nodes[node]->rank1(i);
			node = child1(node);
		}

		level++;

	}

	return c;

}

ulint WaveletTree::size(){//returns size of the structure in bits

	ulint size = 0;

	for(uint i=0;i<number_of_nodes;i++)
		size += nodes[i]->size();

	return size;

}

void WaveletTree::freeMemory(){

	for(ulint i=0;i<number_of_nodes;i++)
		nodes[i]->freeMemory();

	delete [] nodes;

}

void WaveletTree::saveToFile(FILE *fp){

	fwrite(&n, sizeof(ulint), 1, fp);
	fwrite(&sigma, sizeof(uint), 1, fp);
	fwrite(&log_sigma, sizeof(uint), 1, fp);
	fwrite(&number_of_nodes, sizeof(ulint), 1, fp);

	for(ulint i=0;i<number_of_nodes;i++)
		nodes[i]->saveToFile(fp);


}

#define check_numBytes() if (numBytes == 0) { VERBOSE_CHANNEL << "Read 0 bytes when reading dB-hash file (WaveletTree error)" << endl << flush; exit(1); }

void WaveletTree::loadFromFile(FILE *fp){

	ulint numBytes;

	numBytes = fread(&n, sizeof(ulint), 1, fp);
	check_numBytes();
	numBytes = fread(&sigma, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&log_sigma, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&number_of_nodes, sizeof(ulint), 1, fp);
	check_numBytes();

	nodes = new StaticBitVector*[number_of_nodes];

	for(ulint i=0;i<number_of_nodes;i++)
		nodes[i] = new StaticBitVector();

	for(ulint i=0;i<number_of_nodes;i++)
		nodes[i]->loadFromFile(fp);

}

WaveletTree::~WaveletTree() {}

} /* namespace data_structures */
