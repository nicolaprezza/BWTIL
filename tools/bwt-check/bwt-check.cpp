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
 * bwt-check.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../data_structures/IndexedBWT.h"
#include "../../data_structures/FileReader.h"
#include "../../data_structures/BackwardFileIterator.h"


using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 3){
		cout << "*** BWT check ***\n";
		cout << "Given a bwt file and a text file, checks if the former is the valid bwt of the latter\n";
		cout << "Usage: bwt-check bwt_file text_file\n";
		cout << "where:\n";
		cout <<	"- bwt_file is the bwt of text_file, with a 0x0 byte as terminator character (must appear only once in the bwt!).\n";
		cout <<	"- text_file is the plain text file whose bwt is supposed to be stored in bwt_file.\n";
		exit(0);
	}

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;

    auto t1 = high_resolution_clock::now();

    IndexedBWT idxBWT;
    ulint n_inv_bwt=0;

    {

		FileReader fr(argv[1]);
		string bwt = fr.toString();
		fr.close();

		n_inv_bwt = bwt.length()-1;

		cout << "BWT length =  " << bwt.length() << endl;
		cout << "Indexing the BWT ... " << endl << endl;

		//second arg is offrate of SA pointers. If 0, no SA pointers are stored.
		idxBWT = IndexedBWT(bwt,0,true);

    }

    string path = string(argv[2]);
	auto bfr = BackwardFileIterator(path);

	if(bfr.length() != n_inv_bwt){

		cout << "\nError: the bwt file and the text input file do not have same length (excluding bwt terminator)\n";
		cout << "text length = " << bfr.length() << ", bwt length (without terminator) = " << n_inv_bwt << endl;
		cout << argv[1] << " " << "is not a valid BWT of " << argv[2] << endl;
		exit(0);

	}

	cout << "\nDone. Inverting the BWT and checking correctness ... " << endl;

	//invert the bwt

	ulint i=0;//number of steps
	ulint bwt_pos=0;//position in the L column of the BWT.

	int perc, last_perc=-1;

	uchar x,y;

	while(i<n_inv_bwt){

		if((x=idxBWT.at(bwt_pos)) != (y=bfr.read())){

			cout << "\nError: text file and inverted bwt do not match at position " << n_inv_bwt-i-1 << ".\n";
			cout << x << " " << y << " " << (uint)x << " " << (uint)y <<endl;
			cout << argv[1] << " " << "is not a valid BWT of " << argv[2] << endl;
			exit(0);

		}

		bwt_pos = idxBWT.LF(bwt_pos);
		i++;

		perc = (i*100)/(n_inv_bwt-1);
		if(perc>last_perc and perc%10==0){

			cout << perc << "% done"<<endl;
			last_perc=perc;

		}

	}

	bfr.close();

	cout << "\nSUCCESS!\n";
	cout << argv[1] << " " << "is the BWT of " << argv[2] << endl;

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



