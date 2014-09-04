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

/*
 * succinctFMIndex.h
 *
 *  Created on: Jul 23, 2014
 *      Author: nicola
 *
 *  Description: an uncompressed (yet succinct) wavelet-tree based FM index.
 *
 */

#ifndef SUCCINCTFMINDEX_H_
#define SUCCINCTFMINDEX_H_

#include "IndexedBWT.h"
#include "FileReader.h"
#include "../algorithms/cw_bwt.h"

namespace bwtil {

class succinctFMIndex {

public:

	succinctFMIndex(){};

	succinctFMIndex(string text, ulint n, bool verbose= false){

		build(text,verbose);

	}

	succinctFMIndex(string path, bool verbose= false){

		FileReader fr = FileReader(path);
		string text = fr.toString();
		fr.close();

		build(text,verbose);

	}

	ulint size(){//returns size of the structure in bits

		return idxBWT.size();

	}

	vector<ulint> getOccurrencies(string P){

		return idxBWT.convertToTextCoordinates( idxBWT.BS(P) );

	}

	void saveToFile(string path){

		FILE *fp;

		if ((fp = fopen(path.c_str(), "wb")) == NULL) {
			VERBOSE_CHANNEL<< "Cannot open file " << path<<endl;
			exit(1);
		}

		saveToFile(fp);

		fclose(fp);

	}
	void load(string path){

		FILE *fp;

		if ((fp = fopen(path.c_str(), "rb")) == NULL) {
			VERBOSE_CHANNEL<< "Cannot open file "  << path<<endl;
			exit(1);
		}

		loadFromFile(fp);

		fclose(fp);

	}
	void saveToFile(FILE *fp){

		fwrite(&n, sizeof(ulint), 1, fp);
		fwrite(&sigma, sizeof(uint), 1, fp);
		fwrite(&log_sigma, sizeof(uint), 1, fp);
		fwrite(&offrate, sizeof(ulint), 1, fp);

		idxBWT.saveToFile(fp);

	}

	void loadFromFile(FILE *fp){

		ulint numBytes;

		numBytes = fread(&n, sizeof(ulint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&sigma, sizeof(uint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&log_sigma, sizeof(uint), 1, fp);
		assert(numBytes>0);
		numBytes = fread(&offrate, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		idxBWT.loadFromFile(fp);

		numBytes++;//avoids "variable not used" warning

	}

	static succinctFMIndex loadFromFile(string path){

		succinctFMIndex fmi = succinctFMIndex();
		fmi.load(path);
		return fmi;

	}

	ulint textLength(){return n;};

private:

	void build(string text, bool verbose= false){

		this->n=text.length();

		//compute the BWT

		string bwt;

		{

			if(verbose) cout << " Computing the BWT ... " << flush;
			auto cwbwt = cw_bwt(text,cw_bwt::text,true);
			bwt = cwbwt.toString();
			if(verbose) cout << "done." << endl;

		}

		//detect alphabet size
		sigma=0;
		vector<bool> symbols(256,false);
		for(uint i=0;i<n;i++)
			if(not symbols.at(text[i])){

				sigma++;
				symbols.at(text[i]) = true;

			}

		log_sigma = ceil(log2(sigma));
		if(log_sigma==0)
			log_sigma=1;

		uint log_n = ceil(log2(n));
		double epsilon = 0.1;

		offrate = ceil( pow(log_n,1+epsilon)/(double)log_sigma );//offrate = log^(1+epsilon) n / log sigma

		idxBWT = IndexedBWT(bwt,offrate,verbose);

	}

	IndexedBWT idxBWT;
	ulint n;//text length (excluded terminator character 0x0)

	uint sigma;//alphabet size
	uint log_sigma;//log2(sigma)

	ulint offrate;

};

} /* namespace bwtil */
#endif /* SUCCINCTFMINDEX_H_ */
