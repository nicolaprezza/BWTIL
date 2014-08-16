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

	/*
	 * constructor: takes as input BWT where terminator character is 0 and builds structures.
	 * n = length of BWT included terminator character
	 */
	IndexedBWT(unsigned char * BWT, ulint n, ulint offrate, bool verbose){

		this->n=n;
		this->offrate=offrate;

		number_of_SA_pointers = (offrate==0?0:n/offrate + 1);

		if(verbose) cout << " Building indexed BWT data structure" << endl;
		if(verbose) cout << "  Number of sampled SA pointers = " << number_of_SA_pointers << endl;

		w = ceil(log2(n));
		if(w<1) w=1;

		ulint nr_of_terminators=0;

		remapping = vector<uchar>(256,0);

		//compute re-mapping to keep alphabet size to a minimum

		//detect alphabet

		vector<uchar> alphabet;
		vector<bool> char_inserted = vector<bool>(256,false);

		for(ulint i=0;i<n;i++){

			if(BWT[i]==0){//found terminator. Save position
				terminator_position = i;
				nr_of_terminators++;
			}else{

				if(not char_inserted.at(BWT[i])){

					alphabet.push_back(BWT[i]);
					char_inserted.at(BWT[i])=true;

				}

			}
		}

		if(nr_of_terminators!=1){

			cout << "Error (IndexedBWT.cpp): the bwt contains no o more than one 0x0 bytes\n";
			exit(1);

		}

		sigma = alphabet.size();

		//sort alphabet

		std::sort(alphabet.begin(),alphabet.end());

		//calculate remapping

		for(uint i=0;i<alphabet.size();i++)
			remapping[alphabet.at(i)] = i;

		//calculate inverse remapping

		inverse_remapping = vector<uchar>(sigma);

		for(uint i=0;i<sigma;i++)
			inverse_remapping[i] = alphabet.at(i);

		//apply remapping

		for(ulint i=0;i<n;i++)
			BWT[i] = remapping[BWT[i]];//note: remapping of teminator (0x0) is 0

		bwt_wt =  WaveletTree(BWT,n,verbose);

		if(offrate>0)
			marked_positions =  StaticBitVector(n);
		else
			marked_positions =  StaticBitVector(0);

		text_pointers =  WordVector(number_of_SA_pointers,w);

		log_sigma = bwt_wt.bitsPerSymbol();

		TERMINATOR = 256;

		FIRST = vector<ulint>(257,0);

		FIRST[TERMINATOR]=0;//first occurrence of terminator char in the first column is at the beginning

		for(ulint i=0;i<n;i++)
			if(i!=terminator_position)
				FIRST[bwt_wt.charAt(i)]++;

		for(uint i=1;i<256;i++)
			FIRST[i] += FIRST[i-1];

		for(int i=255;i>0;i--)
			FIRST[i] = FIRST[i-1];

		FIRST[0] = 0;

		for(uint i=0;i<256;i++)
			FIRST[i]++;

		if(offrate>0){
			if(verbose) cout << "\n  Marking positions containing a SA pointer ... ";
			markPositions(verbose);
			if(verbose) cout << "  Done.\n";

			if(verbose) cout << "\n  Sampling SA pointers ... ";
			sampleSA(verbose);
			if(verbose) cout << "  Done.\n";
		}

		for(ulint i=0;i<n;i++)
			BWT[i] = inverse_remapping[BWT[i]];//restore original values

	}

	ulint convertToTextCoordinate(ulint i){//i=address on BWT (F column). returns corresponding address on text

		ulint l = 0;//number of LF steps

		while(marked_positions.bitAt(i) == 0){
			i = LF(i);
			l++;

			if(l>n){//prevents loop in case of errors in the BWT
				cout << "Error: loop while scanning BWT. Check input BWT file.\n";
				exit(1);
			}

		}

		//here marked_positions.bitAt(i) == 1

		return text_pointers.wordAt( marked_positions.rank1(i) ) + l;

	}

	vector<ulint> convertToTextCoordinates(pair<ulint, ulint> interval){

		vector<ulint> coord;
		for(uint i=interval.first;i<interval.second;i++)
			coord.push_back( convertToTextCoordinate(i) );

		return coord;

	}

	unsigned char at(ulint i){

		if(i==terminator_position)
			return 0;

		return inverse_remapping[bwt_wt.charAt(i)];

	}

	ulint LF(ulint i){//LF mapping from last column to first

		unsigned char c = charAt_remapped(i);

		return  FIRST[c] + rank(c,i);

	}

	ulint size(){//returns size of the structure in bits

		ulint FIRST_size = (sigma+1)*64;

		return bwt_wt.size() + marked_positions.size() + text_pointers.size() + FIRST_size;

	}

	/*
	 *  To be used with the dB-hash data structure
	 * 	search the suffix of length 'length' of the word W, where each character is formed by 'log_sigma' bits
	 *
	 */
	pair<ulint, ulint> BS(ulint W, uint length,pair<ulint, ulint> interval= pair<ulint, ulint>(0,0)){

		if(interval.first==0 and interval.second==0)//if default interval
			interval.second = n;

		for(uint i=0;i<length;i++){

			uint c = remapping[digitAt(W,i)+1];//sum 1 since the BWT is built on the remapped text, where 1 is added to each digit

			interval.first = FIRST[c] + rank(c,interval.first);
			interval.second = FIRST[c] + rank(c,interval.second);

		}

		return interval;

	}

	/*
	 * 	backward search: search pattern P and return interval <lower_included, upper_excluded> on the BWT
	 *
	 */
	pair<ulint, ulint> BS(string P){

		pair<ulint, ulint> interval = pair<ulint, ulint>(0,n);

		for(uint i=0;i<P.length();i++){

			uint c = (unsigned char)P.at( (P.length()-1)-i );

			if(c==0){
				cout << "ERROR while searching pattern in the index: the pattern contains a 0x0 byte (not allowed since it is used as text terminator).\n";
				exit(0);
			}

			c = remapping[c];//apply remapping

			interval.first = FIRST[c] + rank(c,interval.first);
			interval.second = FIRST[c] + rank(c,interval.second);

		}

		return interval;

	}

	~IndexedBWT() {}

	void freeMemory(){

		bwt_wt.freeMemory();
		marked_positions.freeMemory();
		text_pointers.freeMemory();

	}

	void saveToFile(FILE *fp){

		fwrite(&TERMINATOR, sizeof(uint), 1, fp);
		fwrite(&sigma, sizeof(uint), 1, fp);
		fwrite(&log_sigma, sizeof(uint), 1, fp);
		fwrite(&terminator_position, sizeof(ulint), 1, fp);
		fwrite(&offrate, sizeof(ulint), 1, fp);
		fwrite(&number_of_SA_pointers, sizeof(ulint), 1, fp);
		fwrite(&w, sizeof(uint), 1, fp);
		fwrite(&n, sizeof(ulint), 1, fp);

		bwt_wt.saveToFile(fp);
		marked_positions.saveToFile(fp);
		text_pointers.saveToFile(fp);

		fwrite(FIRST.data(), sizeof(ulint), 257, fp);
		fwrite(remapping.data(), sizeof(uchar), 256, fp);
		fwrite(inverse_remapping.data(), sizeof(uchar), sigma, fp);

	}

	void loadFromFile(FILE *fp){

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

		bwt_wt =  WaveletTree();
		marked_positions =  StaticBitVector();
		text_pointers =  WordVector();

		bwt_wt.loadFromFile(fp);
		marked_positions.loadFromFile(fp);
		text_pointers.loadFromFile(fp);

		FIRST = vector<ulint>(257);
		remapping = vector<uchar>(256);
		inverse_remapping = vector<uchar>(sigma);

		numBytes = fread(FIRST.data(), sizeof(ulint), 257, fp);
		check_numBytes();
		numBytes = fread(remapping.data(), sizeof(uchar), 256, fp);
		check_numBytes();
		numBytes = fread(inverse_remapping.data(), sizeof(uchar), sigma, fp);
		check_numBytes();

	}

	ulint length(){return n;}

	uint TERMINATOR;//terminator character

private:

	unsigned char charAt_remapped(ulint i){

		if(i==terminator_position)
			return TERMINATOR;

		return bwt_wt.charAt(i);

	}

	/*
	 * input: already remapped character (or TERMINATOR) and position
	 * output: rank of the character in the bwt
	 */
	ulint rank(unsigned char c, ulint i){//number of characters 'c' before position i excluded

		if(c==TERMINATOR)
			return i>terminator_position;

		if(c==0 and i>terminator_position)//this because the terminator in the wavelet tree is encoded as 0
			return bwt_wt.rank(0,i)-1;

		return bwt_wt.rank(c,i);

	}

	//returns i-th digit of log_sigma bits from right in the word W
	uint digitAt(ulint W, uint i){

		uint mask = (1<<log_sigma)-1;

		return (W>>(log_sigma*i)) & mask;

	}

	void markPositions(bool verbose){//mark 1 every offrate positions of the text on the bwt (vector marked_positions)

		ulint i=n-1;//current position on text
		ulint j=0;  //current position on the BWT (0=terminator position on the F column)
		uint perc;
		uint last_perc=1;

		if(verbose) cout << endl;

		while(i>0){

			perc = (100*(n-i))/n;

			if(verbose)
				if(perc%10==0 and perc!= last_perc){

					cout << "   " << perc << "% done.\n";
					last_perc=perc;

				}

			if(i%offrate==0)
				marked_positions.setBit(j,1);

			j = LF(j);

			i--;

		}

		//i=0
		marked_positions.setBit(j,1);

		marked_positions.computeRanks();

	}

	void sampleSA(bool verbose){//sample Suffix Array pointers (1 every offrate positions on text). Vector marked_positions must be already computed!

		ulint i=n-1;//current position on text
		ulint j=0;  //current position on the BWT (0=terminator position on the F column)
		uint perc;
		uint last_perc=1;

		if(verbose) cout << endl;

		while(i>0){

			perc = (100*(n-i))/n;

			if(verbose)
				if(perc%10==0 and perc!= last_perc){

					cout << "   " << perc << "% done.\n";
					last_perc=perc;

				}

			if(marked_positions.bitAt(j)==1)
				text_pointers.setWord( marked_positions.rank1(j), i );

			j = LF(j);
			i--;

		}

		//i=0
		text_pointers.setWord( marked_positions.rank1(j), 0 );


	}

	uint sigma;//alphabet size (excluded terminator character)
	uint log_sigma;//number of bits of each character

	ulint n;//BWT length (included terminator character)
	ulint terminator_position;//position of the terminator character in the BWT
	ulint offrate;//distance on the text between 2 marked positions

	ulint number_of_SA_pointers;

	uint w;//size of a pointer = log2 n

	WaveletTree bwt_wt;//BWT stored as a wavelet tree
	StaticBitVector marked_positions;//marks positions on the BWT having a text-pointer
	WordVector text_pointers;//stores sampled text pointers (SA pointers)

	vector<ulint> FIRST;//first column in the matrix of the ordered suffixes. FIRST[c]=position of the first occurrence of c in the 1st column

	vector<uchar> remapping;//from file's chars to {0,...,sigma-1}
	vector<uchar> inverse_remapping;

};

} /* namespace data_structures */
#endif /* INDEXEDBWT_H_ */
