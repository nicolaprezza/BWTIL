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
 * bwt-to-sa.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../data_structures/FileReader.h"

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

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;

    auto t1 = high_resolution_clock::now();

	string bwt_path = string(argv[2]).append(".bwt");

	if(argc==4){

		bwt_path = string(argv[3]);

	}

	cout << "\nReading the text ... " << endl;

	FileReader text_fr(argv[2]);
	ulint n = text_fr.size();
	string text = text_fr.toString();
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

	auto t2 = high_resolution_clock::now();
	ulint total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();

	if(total>=3600){

		uint h = total/3600;
		uint m = (total%3600)/60;
		uint s = (total%3600)%60;

		cout << "Total time: " << h << "h " << m << "m " << s << "s" << endl;

	}else if (total>=60){

		uint m = total/60;
		uint s = total%60;

		cout << "Total time: " << m << "m " << s << "s" << endl;

	}else{

		cout << "Total time: " << total << "s" << endl;

	}
 }



