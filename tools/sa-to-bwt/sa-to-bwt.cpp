/*
 * bwt-to-sa.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../common/common.h"
#include "../../data_structures/FileReader.h"
#include "../../extern/getRSS.h"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 3 and argc != 4){
		cout << "*** Suffix Array to BWT converter ***\n";
		cout << "Given a suffix array file and a text file, builds the BWT and stores it directly to disk.\n";
		cout << "The output BWT file will contain a 0x0 byte as text terminator.\n";
		cout << "Usage: sa-to-bwt sa_file text_file [output_bwt_file]\n";
		cout << "where:\n";
		cout << "- sa_file is the input suffix array file. This file must contain n pointers to the text, each of size 8 bytes.\n";
		cout << "- text_file is the input text file. Input file must not contain a 0x0 byte since the algorithm uses it as text terminator.\n";
		cout << "- output_bwt_file (optional) is the output bwt file. Default: text_file.bwt\n";
		exit(0);
	}

	string bwt_path = string(argv[2]).append(".bwt");

	if(argc==4){

		bwt_path = string(argv[3]);

	}

	cout << "\nReading the text ... " << endl;

	FileReader text_fr(argv[2]);

	ulint n = text_fr.size();
	uchar * text = new uchar[n];//with text terminator 0x0

	text_fr.read(text,n);
	text_fr.close();

	cout << "Done. Reading suffix array and building BWT ... " << endl;

	FILE *fp_sa;
	FILE *fp_bwt;

	if ((fp_sa = fopen(argv[1], "rb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file "  << argv[1] <<endl;
		exit(1);
	}
	if ((fp_bwt = fopen(bwt_path.c_str(), "wb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file "  << bwt_path <<endl;
		exit(1);
	}

	int bytes;

	uchar symbol = text[n-1];

	bytes = fwrite(&symbol, sizeof(uchar), 1, fp_bwt);

	if(bytes<=0){

		cout << "Error while writing bwt file.\n";
		exit(1);

	}

	ulint addr;
	for(ulint i=0;i<n;i++){

		bytes = fread(&addr, sizeof(ulint), 1, fp_sa);

		if(addr>=n){

			cout << "Error: read address greater than text length : " << addr << ">=" << n << " in position " << i << endl;
			exit(1);

		}

		if(bytes<=0){

			cout << "Error: premature end of the SA file\n";
			exit(1);

		}

		if(addr==0){

			symbol = 0;//terminator

		}else{

			symbol = text[addr-1];

		}

		bytes = fwrite(&symbol, sizeof(uchar), 1, fp_bwt);

		if(bytes<=0){

			cout << "Error while writing bwt file.\n";
			exit(1);

		}

	}

	cout << "Done. BWT stored in " << bwt_path << endl;

	fclose(fp_bwt);
	fclose(fp_sa);

	printRSSstat();

 }



