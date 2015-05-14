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

#include "../../data_structures/FileReader.h"
#include "../../data_structures/fid_cgap.h"
#include "set"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 2){
		cout << "*** test fid_cgap structure: converts each run of the input file in a bit run 00...001 and builds the cgap FID on the resulting bitvector. This code serves just for testing purposes***\n";
		cout << "Usage: fid-cgap-test text_file\n";
		exit(0);
	}

	string path = string(argv[1]);
	FileReader text(path);
	ulint length = text.size();

	vector<bool> B;

	uchar last_char = text.get();

	while(not text.eof()){

		uchar c = text.get();

		if(c!=last_char){

			B.push_back(true);
			last_char=c;

		}else{
			B.push_back(false);
		}

	}

	text.close();

	//build FID
	fid_cgap fid(B);

	//print stats
	cout << "Size of FID (Bytes) = " << fid.bytesize() << endl;
	cout << "n (bits set) = " << fid.number_of_1() << endl;
	cout << "u (bitvector length) = " << fid.size() << endl << endl;

	cout << "D bytesize = " << (fid.D_bytesize()) << endl;
	cout << "C bytesize = " << (fid.C_bytesize()) << endl;
	cout << "C addr bytesize = " << (fid.C_addr_bytesize()) << endl;
	cout << "first_el bytesize = " << (fid.first_el_bytesize()) << endl;
	cout << "prefix length = " << fid.get_prefix_length() << endl<<endl;

	cout << "bits per element = " << (double)(fid.bytesize()*8)/fid.number_of_1() << endl;
	cout << "bits per element C = " << (double)(fid.C_bytesize()*8)/fid.number_of_1() << endl;
	cout << "bits per element D = " << (double)(fid.D_bytesize()*8)/fid.number_of_1() << endl;
	cout << "Entropy (bits) = " << fid.entropy() << endl;
	cout << "compression ratio = " << (double)(fid.bytesize()*8)/fid.size() << endl<<endl;

 }

