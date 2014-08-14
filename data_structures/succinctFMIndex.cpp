/*
 * succinctFMIndex.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: nicola
 */

#include "succinctFMIndex.h"

namespace bwtil {

succinctFMIndex::succinctFMIndex(unsigned char * text, ulint n, bool verbose){

	build(text,n,verbose);

}

succinctFMIndex::succinctFMIndex(string path, bool verbose){

	FileReader fr(path);

	unsigned char * text = new unsigned char[fr.size()];

	fr.read(text,fr.size());

	fr.close();

	build(text,fr.size(),verbose);

}

void succinctFMIndex::build(unsigned char * text, ulint n, bool verbose){

	this->n=n;

	text = (unsigned char *)realloc(text,(n+1)*sizeof(unsigned char));
	text[n] = (unsigned char)0;//append 0x0 byte terminator

	//compute the BWT
	if(verbose) cout << " Computing the BWT ... " << flush;
	unsigned char * bwt = computeBWT(text, n+1);
	if(verbose) cout << "done." << endl;

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

	idxBWT = IndexedBWT(bwt,n+1,offrate,verbose);

}


unsigned char * succinctFMIndex::computeBWT(unsigned char* text, ulint length){

	if( text[length-1]!=0 ){

		cerr << "ERROR in BWT computation: input text does not terminate with 0x0 byte.";
		exit(0);

	}

	if( length > ((ulint)1<<31) ){

		cerr << "ERROR in BWT computation: currently SFM-index supports only creation of the BWT for texts of length <= 2^31\n";
		exit(1);

	}

	//suffix array
	int* SA = new int[length];

	if (sais(text, SA, length) != 0) {
		cerr << "Error in the creation of the suffix array with sais library.\n";
		exit(1);
	}

	//compute BWT(T_h)
	unsigned char *bwt = new unsigned char[length];

	for (unsigned int i = 0; i < length; i++) {
		if (SA[i] == 0) 	//first position: previous character is the terminator symbol (#)
			bwt[i] = 0;
		else
			bwt[i] = text[SA[i] - 1];
	}

	delete [] SA;

	return bwt;

}

ulint succinctFMIndex::size(){//returns size of the structure in bits

	return idxBWT.size();

}

vector<ulint> succinctFMIndex::getOccurrencies(string P){

	return idxBWT.convertToTextCoordinates( idxBWT.BS(P) );

}

void succinctFMIndex::saveToFile(string path){

	FILE *fp;

	if ((fp = fopen(path.c_str(), "wb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file " << path<<endl;
		exit(1);
	}

	saveToFile(fp);

}
void succinctFMIndex::load(string path){
	FILE *fp;

	if ((fp = fopen(path.c_str(), "rb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file "  << path<<endl;
		exit(1);
	}

	loadFromFile(fp);

}
void succinctFMIndex::saveToFile(FILE *fp){

	fwrite(&n, sizeof(ulint), 1, fp);
	fwrite(&sigma, sizeof(uint), 1, fp);
	fwrite(&log_sigma, sizeof(uint), 1, fp);
	fwrite(&offrate, sizeof(ulint), 1, fp);

	idxBWT.saveToFile(fp);

}

void succinctFMIndex::loadFromFile(FILE *fp){

	ulint numBytes;

	numBytes = fread(&n, sizeof(ulint), 1, fp);
	check_numBytes();
	numBytes = fread(&sigma, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&log_sigma, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&offrate, sizeof(ulint), 1, fp);
	check_numBytes();

	idxBWT.loadFromFile(fp);

}


succinctFMIndex succinctFMIndex::loadFromFile(string path){

	succinctFMIndex fmi = succinctFMIndex();
	fmi.load(path);
	return fmi;

}

} /* namespace bwtil */
