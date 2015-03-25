/*
 *  This file is part of BWTIL.
 *  Copyright (c) by
 *  Nicola Prezza <nicolapr@gmail.com>
 *
 *   BWTIL is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   BWTIL is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details (<http://www.gnu.org/licenses/>).
 */

//============================================================================
// Name        : IndexedBWT.h
// Author      : Nicola Prezza
// Version     : 1.0
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
#include "succinct_bitvector.h"

namespace bwtil {

class IndexedBWT {
public:

	IndexedBWT(){};

	/*
	 * constructor: takes as input BWT where terminator character is 0 and builds structures.
	 */
	IndexedBWT(string &BWT, ulint sample_rate, bool verbose=false){

		this->n=BWT.length();
		this->offrate=sample_rate;

		number_of_SA_pointers = (sample_rate==0?0:n/sample_rate + 1);

		if(verbose) cout << " Building indexed BWT data structure" << endl;
		if(verbose) cout << "  Number of sampled SA pointers = " << number_of_SA_pointers << endl;

		w = ceil(log2(n));
		if(w<1) w=1;

		ulint nr_of_terminators=0;

		//compute re-mapping to keep alphabet size to a minimum
		//from text chars -> to integers in {0,...,sigma}. the 0x0 byte is also remapped in 0x0, as the first alphabet character.
		remapping = vector<uchar>(256,0);

		//detect alphabet
		vector<uchar> alphabet;
		vector<bool> char_inserted = vector<bool>(256,false);

		for(ulint i=0;i<n;i++){

			if((uchar)BWT.at(i)==0){//found terminator. Save position
				terminator_position = i;
				nr_of_terminators++;
			}else{

				if(not char_inserted.at((uchar)BWT.at(i))){

					alphabet.push_back((uchar)BWT.at(i));
					char_inserted.at((uchar)BWT.at(i))=true;

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
		//note: remapping of terminator (0x0) is 0

		for(uint i=0;i<alphabet.size();i++)
			remapping[alphabet.at(i)] = i;

		//calculate inverse remapping

		inverse_remapping = vector<uchar>(sigma);

		for(uint i=0;i<sigma;i++)
			inverse_remapping[i] = alphabet.at(i);

		//apply remapping

		for(ulint i=0;i<n;i++)
			BWT.at(i) = remapping[(uchar)BWT.at(i)];

		bwt_wt =  WaveletTree(BWT,verbose);

		marked_positions =  succinct_bitvector();

		text_pointers =  packed_view_t(w,number_of_SA_pointers);

		log_sigma = bwt_wt.bitsPerSymbol();

		FIRST = vector<ulint>(256,0);

		FIRST[TERMINATOR]=0;//first occurrence of terminator char in the first column is at the beginning

		//count number of occurrences of each character
		for(ulint i=0;i<n;i++)
			if(i!=terminator_position)
				FIRST[bwt_wt.charAt(i)]++;

		for(uint i=1;i<255;i++)
			FIRST[i] += FIRST[i-1];

		for(int i=254;i>0;i--)
			FIRST[i] = FIRST[i-1];

		FIRST[0] = 0;

		for(uint i=0;i<255;i++)
			FIRST[i]++;

		//restore original values in BWT
		for(ulint i=0;i<n;i++)
			BWT.at(i) = inverse_remapping[(uchar)BWT.at(i)];

		BWT.at(terminator_position) = 0;

		if(sample_rate>0){
			if(verbose) cout << "\n  Marking positions containing a SA pointer ... ";

			vector<bool> mark_pos = markPositions(verbose);
			marked_positions = succinct_bitvector( mark_pos );

			if(verbose) cout << "  Done.\n";

			if(verbose) cout << "\n  Sampling SA pointers ... ";
			sampleSA(verbose);
			if(verbose) cout << "  Done.\n";
		}

	}

	ulint convertToTextCoordinate(ulint i){//i=address on BWT (F column). returns corresponding address on text

		ulint l = 0;//number of LF steps

		while(marked_positions.at(i) == 0){
			i = LF(i);
			l++;

			if(l>n){//prevents loop in case of errors in the BWT
				cout << "Error: loop while scanning BWT. Check input BWT file.\n";
				exit(1);
			}

		}

		//here marked_positions.bitAt(i) == 1

		return text_pointers[marked_positions.rank1(i)] + l;

	}

	vector<ulint> convertToTextCoordinates(pair<ulint, ulint> interval){

		vector<ulint> coord;
		for(uint i=interval.first;i<interval.second;i++)
			coord.push_back( convertToTextCoordinate(i) );

		return coord;

	}

	uchar at(ulint i){

		if(i==terminator_position)
			return 0;

		return inverse_remapping[bwt_wt.charAt(i)];

	}

	ulint LF(ulint i){//LF mapping from last column to first

		uchar c = charAt_remapped(i);
		return  FIRST[c] + rank(c,i);

	}

	ulint size(){//returns size of the structure in bits

		ulint FIRST_size = (sigma+1)*64;

		return bwt_wt.size() + marked_positions.size() + text_pointers.size()*text_pointers.width() + FIRST_size;

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

			auto c = remapping[digitAt(W,i)+1];//sum 1 since the BWT is built on the remapped text, where 1 is added to each digit

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

			auto c = (uchar)P.at( (P.length()-1)-i );

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

	void saveToFile(FILE *fp){

		fwrite(&sigma, sizeof(uint), 1, fp);
		fwrite(&log_sigma, sizeof(uint), 1, fp);
		fwrite(&terminator_position, sizeof(ulint), 1, fp);
		fwrite(&offrate, sizeof(ulint), 1, fp);
		fwrite(&number_of_SA_pointers, sizeof(ulint), 1, fp);
		fwrite(&w, sizeof(uint), 1, fp);
		fwrite(&n, sizeof(ulint), 1, fp);

		bwt_wt.saveToFile(fp);
		marked_positions.saveToFile(fp);
		save_packed_view_to_file(text_pointers,number_of_SA_pointers,fp);

		fwrite(FIRST.data(), sizeof(ulint), 256, fp);
		fwrite(remapping.data(), sizeof(uchar), 256, fp);
		fwrite(inverse_remapping.data(), sizeof(uchar), sigma, fp);

	}

	void loadFromFile(FILE *fp){

		ulint numBytes;

		numBytes = fread(&sigma, sizeof(uint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&log_sigma, sizeof(uint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&terminator_position, sizeof(ulint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&offrate, sizeof(ulint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&number_of_SA_pointers, sizeof(ulint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&w, sizeof(uint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&n, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		bwt_wt =  WaveletTree();
		bwt_wt.loadFromFile(fp);

		marked_positions =  succinct_bitvector();
		marked_positions.loadFromFile(fp);
		text_pointers = load_packed_view_from_file(w, number_of_SA_pointers, fp);

		FIRST = vector<ulint>(256);
		remapping = vector<uchar>(256);
		inverse_remapping = vector<uchar>(sigma);

		numBytes = fread(FIRST.data(), sizeof(ulint), 256, fp);
		assert(numBytes>0);
		numBytes = fread(remapping.data(), sizeof(uchar), 256, fp);
		assert(numBytes>0);
		numBytes = fread(inverse_remapping.data(), sizeof(uchar), sigma, fp);
		assert(numBytes>0);

		numBytes++;//avoids "variable not used" warning

	}

	ulint length(){return n;}

private:

	//returns symbol stored in the wavelet tree at position i. The terminator is returned as 255
	uchar charAt_remapped(ulint i){

		if(i==terminator_position)
			return TERMINATOR;

		return bwt_wt.charAt(i);

	}

	/*
	 * input: already remapped character (or TERMINATOR) and position
	 * output: rank of the character in the bwt
	 */
	ulint rank(uchar c, ulint i){//number of characters 'c' before position i excluded

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

	vector<bool> markPositions(bool verbose){//mark 1 every offrate positions of the text on the bwt (vector marked_positions)

		auto marked_pos_vec = vector<bool>(n,false);

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
				marked_pos_vec.at(j)=true;

			j = LF(j);

			i--;

		}

		//i=0
		marked_pos_vec.at(j)=true;
		//marked_positions.setBit(j,1);
		//marked_positions.computeRanks();

		return marked_pos_vec;

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

			if(marked_positions.at(j)==1)
				text_pointers[marked_positions.rank1(j)] = i;

			j = LF(j);
			i--;

		}

		//i=0
		text_pointers[marked_positions.rank1(j)] = 0;

	}

	//terminator character in the remapped text. 255 is never used since 0x0 is not present in the original text and all characters are remapped
	//in the lowest values
	static const uint TERMINATOR = 255;

	uint sigma;//alphabet size (excluded terminator character)
	uint log_sigma;//number of bits of each character

	ulint n;//BWT length (included terminator character)
	ulint terminator_position;//position of the terminator character in the BWT
	ulint offrate;//distance on the text between 2 marked positions

	ulint number_of_SA_pointers;

	uint w;//size of a pointer = log2 n

	WaveletTree bwt_wt;//BWT stored as a wavelet tree
	succinct_bitvector marked_positions;//marks positions on the BWT having a text-pointer
	packed_view_t text_pointers;

	vector<ulint> FIRST;//first column in the matrix of the ordered suffixes. FIRST[c]=position of the first occurrence of c in the 1st column

	vector<uchar> remapping;//from file's chars to {0,...,sigma-1}
	vector<uchar> inverse_remapping;

};

} /* namespace data_structures */
#endif /* INDEXEDBWT_H_ */
