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
#include <sstream>

using namespace bwtil;

void help(){
	cout << "*** Count number of phrases in the LZ77 parse of the file ***\n";
	cout << "Usage: lz77 [options] text_file\n";
	cout << "Options:" <<  endl;
	cout << "--v1 : [default] LZ77 variant 1: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase Wc is inserted in the dictionary."<<endl;
	cout << "--v2 : LZ77 variant 2: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase W is inserted in the dictionary, and c is part of the next phrase. If W=empty, a new phrase 'c' is inserted in the dictionary, and the next phrase is initialized empty."<<endl;
	cout << "--p arg : output the number of phrases every <arg> characters. With this option, output percentages are suppressed."<<endl;
	exit(0);
}

lz77_t::options parse(lz77_t::options &opt, int &ptr, char** argv, int argc){

	string s(argv[ptr]);
	ptr++;

	if(s.compare("--v1")==0){

		if(opt.block<=0)
			opt.verbose=true;

		opt.lz_variant = lz77_t::v1;

	}else if(s.compare("--v2")==0){

		if(opt.block<=0)
			opt.verbose=true;

		opt.lz_variant = lz77_t::v2;

	}else if(s.compare("--p")==0){

		opt.verbose = false;

		if(ptr>=argc){
			cout<<"Missing block size in option --p" << endl;
			help();
		}

		int block_size;
		istringstream ( string(argv[ptr]) ) >> block_size;

		if(block_size<=0){
			cout << "error: block size in --p must be > 0" << endl;
			help();
		}

		opt.block=block_size;

		ptr++;

	}else{
		cout << "Unrecognized option " << s<< endl << endl;
		help();
	}

	return opt;
}

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc < 2)
		help();

	lz77_t::options opt;
	opt.verbose=true;

	int ptr = 1;

	//parse arguments. Last arg is path
	while(ptr<argc-1)
		opt = parse(opt,ptr,argv, argc);


	//if args are correct, now ptr = argc-1 = path
	if(ptr!=argc-1){

		cout << "Error: missing file path." << endl;
		help();

	}

	opt.path = string(argv[ptr]);

	lz77_t lz77(opt);
	cout << endl << endl << "Total number of LZ77 phrases = " << lz77.getNumberOfPhrases() << endl;

 }

