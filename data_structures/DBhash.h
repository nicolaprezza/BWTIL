//============================================================================
// Name        : DBhash.h
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : A dB-hash (de Bruijn hash) data structures on general alphabets and texts of length n<2^64.
//============================================================================

#ifndef DBHASH_H_
#define DBHASH_H_

#include "../common/common.h"
#include "HashFunction.h"
#include "IndexedBWT.h"
#include "../extern/sais.h"

namespace bwtil {

class DBhash {

public:

	DBhash(){};

	DBhash(unsigned char * text, unsigned char * bwt, ulint n, HashFunction h, ulint offrate = 4, bool verbose = false){

		if(verbose)	cout << "\nBuilding dB-hash data structure" <<endl;

		this->n = n;
		this->h = h;
		this->offrate = offrate;

		m = h.m;
		w = h.w;

		text_fingerprint_length = n-m+w+1;

		if(verbose)	cout << " Text fingerprint length = " << text_fingerprint_length<<endl;

		w_aux = ceil( ( log2(n) - log2(log2(n)) )/log2(h.base) );//log_b n - log_b log_2 n
		auxiliary_hash_size = 1 << (w_aux * h.log_base);

		if(verbose)	cout << " w_aux = " << w_aux <<endl;

		if(verbose)	cout << " Building bit tables ...";

		if(verbose)	cout << " Done.\n";

		if(verbose)	cout << " Storing text in plain format ...";

		initText(text);

		if(verbose)	cout << " Done.\n";

		indexedBWT =  IndexedBWT(bwt,text_fingerprint_length,offrate,verbose);

		initAuxHash();

		if(verbose)	cout << "Done. Size of the structure = " << (double)size()/(n*8) << "n Bytes" <<endl;

	}

	DBhash(unsigned char * text, ulint n, HashFunction h, ulint offrate = 4, bool verbose = false){

		if(verbose)	cout << "\nBuilding dB-hash data structure" <<endl;

		this->n = n;
		this->h = h;
		this->offrate = offrate;

		m = h.m;
		w = h.w;

		text_fingerprint_length = n-m+w+1;

		if(verbose) cout << " Text fingerprint length = " << text_fingerprint_length<<endl;

		w_aux = ceil( ( log2(n) - log2(log2(n)) )/log2(h.base) );//log_b n - log_b log_2 n

		auxiliary_hash_size = 1 << (w_aux * h.log_base);

		if(verbose)	cout << " w_aux = " << w_aux <<endl;

		if(verbose)	cout << " Computing hash value h(T) of the text ...";
		unsigned char * fingerprint = h.hashValueRemapped(text,n);
		if(verbose)	cout << " Done.\n";

		if(verbose)	cout << " Storing text T in plain format ...";
		initText(text);
		if(verbose)	cout << " Done.\n";

		if(verbose)	cout << " Computing BWT(h(T))  ...";
		unsigned char * bwt = computeBWT(fingerprint,text_fingerprint_length);
		if(verbose)	cout << " Done.\n";

		delete [] fingerprint;

		indexedBWT =  IndexedBWT(bwt,text_fingerprint_length,offrate,verbose);

		delete [] bwt;

		if(verbose)	cout << "\n  Building auxiliary hash ... " << endl;
		initAuxHash();
		if(verbose)	cout << "  Done. " << endl;

		if(verbose)	cout << "\nDone. Size of the structure = " << (double)size()/(n*8) << "n Bytes" <<endl;

	}

	//doesn't use auxiliary hash.
	vector<ulint> getOccurrencies_slow(ulint fingerprint){

		return indexedBWT.convertToTextCoordinates( indexedBWT.BS(fingerprint,w) );

	}

	//returns occurrences in the text of substrings having 'fingerprint' as hash value. uses the auxiliary hash.
	vector<ulint> getOccurrencies(ulint fingerprint){

		ulint mask = auxiliary_hash_size-1;

		ulint suffix = fingerprint & mask;//suffix of length w_aux. Searched in the auxiliary hash
		ulint prefix = fingerprint >> (w_aux*h.log_base);//prefix of the fingerprint of length log m/log base to be searched with backward search

		pair<ulint, ulint> interval;

		interval.first = auxiliary_hash[suffix];

		if (suffix + 1 == auxiliary_hash_size)
			interval.second = text_fingerprint_length + 1;
		else
			interval.second = auxiliary_hash[suffix+1];

		return indexedBWT.convertToTextCoordinates( indexedBWT.BS(prefix,w-w_aux,interval) );

	}

	ulint size(){//returns size of the structure in bits

		return indexedBWT.size() + text_wv.size()*text_wv.width() + auxiliary_hash.size()*auxiliary_hash.width();

	}

	unsigned char textAt(ulint i){
		return int_to_char[text_wv[i]];
	}

	unsigned char * computeBWT(unsigned char* text, ulint length){

		if( text[length-1]!=0 ){

			cerr << "ERROR in BWT computation: input text does not terminate with 0x0 byte.";
			exit(0);

		}

		if( length > ((ulint)1<<31) ){

			cerr << "ERROR in BWT computation: currently dB-hash supports only creation of the BWT for texts of length <= 2^31\n";
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

	void saveToFile(string path){

		FILE *fp;

		if ((fp = fopen(path.c_str(), "wb")) == NULL) {
			VERBOSE_CHANNEL<< "Cannot open file " << path<<endl;
			exit(1);
		}

		saveToFile(fp);

	}
	void load(string path){
		FILE *fp;

		if ((fp = fopen(path.c_str(), "rb")) == NULL) {
			VERBOSE_CHANNEL<< "Cannot open file "  << path<<endl;
			exit(1);
		}

		loadFromFile(fp);

	}

	void saveToFile(FILE *fp){

		fwrite(&n, sizeof(ulint), 1, fp);
		fwrite(&m, sizeof(ulint), 1, fp);
		fwrite(&w, sizeof(ulint), 1, fp);
		fwrite(&w_aux, sizeof(uint), 1, fp);
		fwrite(&auxiliary_hash_size, sizeof(ulint), 1, fp);
		fwrite(&text_fingerprint_length, sizeof(ulint), 1, fp);
		fwrite(&sigma, sizeof(uint), 1, fp);
		fwrite(&log_sigma, sizeof(uint), 1, fp);

		fwrite(char_to_int.data(), sizeof(uint), 256, fp);
		fwrite(int_to_char.data(), sizeof(unsigned char), sigma, fp);

		h.saveToFile(fp);
		indexedBWT.saveToFile(fp);

		save_packed_view_to_file(text_wv,n,fp);
		save_packed_view_to_file(auxiliary_hash,auxiliary_hash_size,fp);

	}

	void loadFromFile(FILE *fp){

		ulint numBytes;

		numBytes = fread(&n, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&m, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&w, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&w_aux, sizeof(uint), 1, fp);
		check_numBytes();
		numBytes = fread(&auxiliary_hash_size, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&text_fingerprint_length, sizeof(ulint), 1, fp);
		check_numBytes();
		numBytes = fread(&sigma, sizeof(uint), 1, fp);
		check_numBytes();
		numBytes = fread(&log_sigma, sizeof(uint), 1, fp);
		check_numBytes();

		char_to_int = vector<uint>(256);
		int_to_char = vector<uchar>(sigma);

		numBytes = fread(char_to_int.data(), sizeof(uint), 256, fp);
		check_numBytes();
		numBytes = fread(int_to_char.data(), sizeof(unsigned char), sigma, fp);
		check_numBytes();

		h = HashFunction();
		h.loadFromFile(fp);

		indexedBWT =  IndexedBWT();
		indexedBWT.loadFromFile(fp);

		text_wv = load_packed_view_from_file(log_sigma,n,fp);

		auxiliary_hash = load_packed_view_from_file(ceil(log2(n+1)),auxiliary_hash_size,fp);

	}

	static DBhash loadFromFile(string path){

		DBhash dbh = DBhash();
		dbh.load(path);
		return dbh;

	}

	vector<ulint> getOccurrencies(string P, uint max_errors=0){

		if(P.length()!=m){
			cerr << "Error: Searching pattern of length " << P.length() << " in a dB-hash with pattern length " << m<<endl;
			exit(1);
		}

		return filterOutBadOccurrencies(P, getOccurrencies_slow(h.hashValue(P)), max_errors);

	}

	//given a pattern, a list of (candidate) occurrencies and a maximum number of errors (Hamming distance), filter out occurrencies at distance > max_errors
	vector<ulint> filterOutBadOccurrencies(string P, vector<ulint> occ, uint max_errors){

		vector<ulint> good;

		ulint dist=0;
		ulint s;//shift in text

		for(ulint i=0;i<occ.size();i++){

			dist = 0;
			s = occ.at(i);

			for(ulint j=0;j<P.length() and dist<=max_errors;j++)
				if(P.at(j) != textAt(s+j))
					dist++;

			if(dist<=max_errors)
				good.push_back(s);

		}

		return good;

	}

	ulint textLength(){return n;}
	ulint patternLength(){return m;}

	HashFunction hashFunction(){return h;}

protected:

	void initText(unsigned char * text){

		char_to_int = vector<uint>(256);

		ulint empty=256;

		for(ulint i=0;i<256;i++)
			char_to_int[i]=empty;

		sigma=0;

		//compute alphabet size and init codes to convert char to int
		for(ulint i = 0;i<n;i++){

			if(char_to_int[text[i]]==empty){
				char_to_int[text[i]] = sigma;
				sigma++;
			}

		}

		int_to_char = vector<uchar>(sigma);

		for(ulint i=0;i<256;i++){

			if(char_to_int[i]!=empty)
				int_to_char[char_to_int[i]]=i;

		}

		log_sigma = ceil(log2(sigma));
		if(log_sigma==0) log_sigma=1;

		text_wv =  packed_view_t(log_sigma,n);

		for(ulint i = 0;i<n;i++)
			text_wv[i] = char_to_int[text[i]];

	}

	void initAuxHash(){

		auxiliary_hash =  packed_view_t(ceil(log2(n+1)),auxiliary_hash_size);

		ulint empty = text_fingerprint_length+1;

		int perc,last_perc=-1;

		for (ulint i = 0; i < auxiliary_hash_size; i++){

			pair<ulint,ulint> interval = indexedBWT.BS(i,w_aux);

			if(interval.second>interval.first)
				auxiliary_hash[i] = interval.first;
			else
				auxiliary_hash[i] = empty;

			perc = (100*i)/auxiliary_hash_size;
			if(perc>last_perc and perc%10==0){

				cout << "   " <<  perc << "% done." << endl;
				last_perc=perc;

			}

		}

		if (auxiliary_hash[auxiliary_hash_size-1] == empty)
			auxiliary_hash[auxiliary_hash_size-1] = text_fingerprint_length;

		for (ulint i = auxiliary_hash_size - 1; i >= 1; i--) {

			if (auxiliary_hash[i-1] == empty)
				auxiliary_hash[i-1] = auxiliary_hash.get(i);

		}

	}

	ulint n,m,w;//text length, pattern length, word length

	uint w_aux;//base^w_aux = number of entries in the auxiliary hash
	ulint auxiliary_hash_size;//number of entries in the auxiliary hash (base^w_aux)

	ulint text_fingerprint_length;

	HashFunction h;
	ulint offrate;

	IndexedBWT indexedBWT;
	packed_view_t text_wv;//the plain text
	packed_view_t auxiliary_hash;

	uint sigma;//alphabet size
	uint log_sigma;//log2(sigma)

	vector<uint> char_to_int;//conversion from a char in the text to an integer in the range {0,...,sigma-1}
	vector<uchar> int_to_char;//conversion from int in the range {0,...,sigma-1} to a char

};

} /* namespace data_structures */
#endif /* DBHASH_H_ */
