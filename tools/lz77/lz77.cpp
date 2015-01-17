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

#include "../../data_structures/LZ77.h"

using namespace bwtil;

void help(){
	cout << "*** Count number of phrases in the LZ77 parse of the file ***\n";
	cout << "Usage: lz77 [options] text_file\n";
	cout << "Options:" <<  endl;
	cout << "--v1 : [default] LZ77 variant 1: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase Wc is inserted in the dictionary."<<endl;
	cout << "--v2 : LZ77 variant 2: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase W is inserted in the dictionary, and c is part of the next phrase. If W=empty, a new phrase 'c' is inserted in the dictionary, and the next phrase is initialized empty."<<endl;
	exit(0);
}

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 2 and argc != 3)
		help();

	lz77_t::options opt;
	opt.verbose = true;
	opt.lz_variant = lz77_t::v1;

	if(argc==2){

		//--v1 default

		opt.path = string(argv[1]);
		lz77_t lz77(opt);
		cout << endl << "Number of LZ77 phrases = " << lz77.getNumberOfPhrases() << endl;

	}else{

		opt.path = string(argv[2]);
		string s(argv[1]);

		if(s.compare("--v1")==0){

			lz77_t lz77(opt);
			cout << endl << "Number of LZ77 phrases = " << lz77.getNumberOfPhrases() << endl;

		}else if(s.compare("--v2")==0){

			opt.lz_variant = lz77_t::v2;
			lz77_t lz77(opt);
			cout << endl << "Number of LZ77 phrases = " << lz77.getNumberOfPhrases() << endl;

		}else{
			cout << "Unrecognized option " << s<< endl << endl;
			help();
		}

	}

 }

