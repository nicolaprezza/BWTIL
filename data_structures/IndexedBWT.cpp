//============================================================================
// Name        : IndexedBWT.cpp
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : BWT with succinct rank structures and sampled SA pointers
//============================================================================

#include "IndexedBWT.h"

namespace bwtil {

/*
 * constructor: takes as input BWT where terminator character is 0 and builds structures.
 * n = length of BWT included terminator character
 */
IndexedBWT::IndexedBWT(unsigned char * BWT, ulint n, ulint offrate, bool verbose){

	this->n=n;
	this->offrate=offrate;

	number_of_SA_pointers = n/offrate + 1;

	if(verbose) cout << " Building indexed BWT data structure over h(T)" << endl;
	if(verbose) cout << "  Number of sampled SA pointers = " << number_of_SA_pointers << endl;

	w = ceil(log2(n));
	if(w<1) w=1;

	//re-mapping
	for(ulint i=0;i<n;i++){

		if(BWT[i]==0)
			terminator_position = i;
		else
			BWT[i]--;

	}

	bwt_wt = new WaveletTree(BWT,n,verbose);

	marked_positions = new StaticBitVector(n);
	text_pointers = new WordVector(number_of_SA_pointers,w);

	sigma = bwt_wt->alphabetSize();
	log_sigma = bwt_wt->bitsPerSymbol();

	TERMINATOR = sigma;

	FIRST = new ulint [sigma+1];

	for(uint i=0;i<sigma;i++)
		FIRST[i]=0;

	FIRST[TERMINATOR]=0;//first occurrence of terminator char in the first column is at the beginning

	for(ulint i=0;i<n;i++)
		if(i!=terminator_position)
			FIRST[bwt_wt->charAt(i)]++;

	for(uint i=1;i<sigma;i++)
		FIRST[i] += FIRST[i-1];

	for(int i=sigma-1;i>0;i--)
		FIRST[i] = FIRST[i-1];

	FIRST[0] = 0;

	for(uint i=0;i<sigma;i++)
		FIRST[i]++;

	if(verbose) cout << "  Marking positions containing a SA pointer ... ";
	markPositions();
	if(verbose) cout << "Done.\n";

	if(verbose) cout << "  Sampling SA pointers ... ";
	sampleSA();
	if(verbose) cout << "Done.\n";

}

void IndexedBWT::markPositions(){//mark 1 every offrate positions of the text on the bwt (vector marked_positions)

	ulint i=n-1;//current position on text
	ulint j=0;  //current position on the BWT (0=terminator position on the F column)

	while(i>0){

		if(i%offrate==0)
			marked_positions->setBit(j,1);

		j = LF(j);
		i--;

	}

	//i=0
	marked_positions->setBit(j,1);

	marked_positions->computeRanks();

}

void IndexedBWT::sampleSA(){//sample Suffix Array pointers (1 every offrate positions on text). Vector marked_positions must be already computed!

	ulint i=n-1;//current position on text
	ulint j=0;  //current position on the BWT (0=terminator position on the F column)

	while(i>0){

		if(marked_positions->bitAt(j)==1)
			text_pointers->setWord( marked_positions->rank1(j), i );

		j = LF(j);
		i--;

	}

	//i=0
	text_pointers->setWord( marked_positions->rank1(j), 0 );


}

ulint IndexedBWT::convertToTextCoordinate(ulint i){//i=address on BWT (F column). returns corresponding address on text

	ulint l = 0;//number of LF steps

	while(marked_positions->bitAt(i) == 0){
		i = LF(i);
		l++;

		if(l>n){//prevents loop in case of errors in the BWT
			cout << "Error: loop while scanning BWT.\n";
			exit(1);
		}

	}

	//here marked_positions->bitAt(i) == 1

	return text_pointers->wordAt( marked_positions->rank1(i) ) + l;

}

void IndexedBWT::test(){

	cout << "BEGIN TEST\n";

	/*ulint i = n-1;//position on text
	ulint j = 0;//position on BWT

	while(i>0){
		cout << getTextAddress(j) << " ";
		j=LF(j);
		i--;
	}

	cout << getTextAddress(j) << endl;*/

	/*for(ulint i=0;i<n;i++)
		cout << (uint)charAt(i);*/

	cout << "log sigma = " << log_sigma<<endl;
	cout << "number of nodes = " << bwt_wt->numberOfNodes()<<endl;
	cout << "Size of the structure = " << (double)size()/n << "n bits = " << ((double)size()/n)/8 << "n Bytes" <<endl;

	cout << "\nEND TEST\n";

}

vector<ulint> IndexedBWT::convertToTextCoordinates(pair<ulint, ulint> interval){

	vector<ulint> coord;
	for(uint i=interval.first;i<interval.second;i++)
		coord.push_back( convertToTextCoordinate(i) );

	return coord;

}

ulint IndexedBWT::rank(unsigned char c, ulint i){//number of characters 'c' before position i excluded

	if(c==TERMINATOR)
		return i>terminator_position;

	if(c==0 and i>terminator_position)
		return bwt_wt->rank(0,i)-1;

	return bwt_wt->rank(c,i);

}

unsigned char IndexedBWT::at(ulint i){

	if(i==terminator_position)
		return 0;

	return bwt_wt->charAt(i)+1;

}

unsigned char IndexedBWT::charAt_remapped(ulint i){

	if(i==terminator_position)
		return TERMINATOR;

	return bwt_wt->charAt(i);

}

ulint IndexedBWT::LF(ulint i){//LF mapping from last column to first

	unsigned char c = charAt_remapped(i);
	return  FIRST[c] + rank(c,i);

}

ulint IndexedBWT::size(){//returns size of the structure in bits

	ulint FIRST_size = (sigma+1)*64;

	return bwt_wt->size() + marked_positions->size() + text_pointers->size() + FIRST_size;

}

//returns i-th digit of log_sigma bits from right in the word W
uint IndexedBWT::digitAt(ulint W, uint i){

	uint mask = (1<<log_sigma)-1;

	return (W>>(log_sigma*i)) & mask;

}

pair<ulint, ulint> IndexedBWT::BS(ulint W, uint length,pair<ulint, ulint> interval){

	if(interval.first==0 and interval.second==0)//if default interval
		interval.second = n;

	for(uint i=0;i<length;i++){

		uint c = digitAt(W,i);

		interval.first = FIRST[c] + rank(c,interval.first);
		interval.second = FIRST[c] + rank(c,interval.second);

	}

	return interval;

}

IndexedBWT::~IndexedBWT() {}

void IndexedBWT::freeMemory(){

	bwt_wt->freeMemory();
	marked_positions->freeMemory();
	text_pointers->freeMemory();

}

void IndexedBWT::saveToFile(FILE *fp){

	fwrite(&TERMINATOR, sizeof(uint), 1, fp);
	fwrite(&sigma, sizeof(uint), 1, fp);
	fwrite(&log_sigma, sizeof(uint), 1, fp);
	fwrite(&terminator_position, sizeof(ulint), 1, fp);
	fwrite(&offrate, sizeof(ulint), 1, fp);
	fwrite(&number_of_SA_pointers, sizeof(ulint), 1, fp);
	fwrite(&w, sizeof(uint), 1, fp);
	fwrite(&n, sizeof(ulint), 1, fp);

	bwt_wt->saveToFile(fp);
	marked_positions->saveToFile(fp);
	text_pointers->saveToFile(fp);

	fwrite(FIRST, sizeof(ulint), sigma+1, fp);

}

#define check_numBytes() if (numBytes == 0) { VERBOSE_CHANNEL << "Read 0 bytes when reading dB-hash file (IndexedBWT error)" << endl << flush; exit(1); }

void IndexedBWT::loadFromFile(FILE *fp){

	ulint numBytes;

	numBytes = fread(&TERMINATOR, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&sigma, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&log_sigma, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&terminator_position, sizeof(ulint), 1, fp);
	check_numBytes();
	numBytes = fread(&offrate, sizeof(ulint), 1, fp);
	check_numBytes();
	numBytes = fread(&number_of_SA_pointers, sizeof(ulint), 1, fp);
	check_numBytes();
	numBytes = fread(&w, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&n, sizeof(ulint), 1, fp);
	check_numBytes();

	bwt_wt = new WaveletTree();
	marked_positions = new StaticBitVector();
	text_pointers = new WordVector();

	bwt_wt->loadFromFile(fp);
	marked_positions->loadFromFile(fp);
	text_pointers->loadFromFile(fp);

	FIRST = new ulint[sigma+1];

	numBytes = fread(FIRST, sizeof(ulint), sigma+1, fp);
	check_numBytes();


}

} /* namespace data_structures */
