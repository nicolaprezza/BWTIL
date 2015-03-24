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
#include <iostream>
#include <fstream>

using namespace bwtil;

bool print_parse=false;
bool save_parse=false;
string out_filename;
char sep_out;

void help(){
	cout << "*** Count number of phrases in the LZ77 parse of the file ***\n";
	cout << "*** Input file must not contain a 0x0 character, since 0x0 is automatically appended as terminator. ***\n";
	cout << "Usage: lz77 [options] text_file\n";
	cout << "Options:" <<  endl;
	cout << "--v1 : [default] LZ77 variant 1: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase Wc is inserted in the dictionary."<<endl;
	cout << "--v2 : LZ77 variant 2: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase W is inserted in the dictionary, and c is part of the next phrase. If W=empty, a new phrase 'c' is inserted in the dictionary, and the next phrase is initialized empty."<<endl;
	cout << "--p arg : output the number of phrases every <arg> characters."<<endl;
	cout << "--s arg : output the number of phrases each time a character equal to <arg> is encountered. Warning: <arg> characters are skipped and not taken into account in the LZ parse."<<endl;
	cout << "--sample-rate arg : Sample rate to be used in the dynamic FM index. If 0, don't perform sampling and do not print starting positions of phrases. Default: 8."<<endl;
	cout << "--print-parse : [default:false] print the parse"<<endl;
	cout << "--verbose : [default:false] show percentage of work done."<<endl;
	cout << "--save-parse sep filename : saves the LZ factorization in file 'filename', separating each phrase using character 'sep'. " << endl;
	exit(0);
}

lz77_t::options parse(lz77_t::options &opt, int &ptr, char** argv, int argc){

	string s(argv[ptr]);
	ptr++;

	if(s.compare("--v1")==0){

		opt.lz_variant = lz77_t::v1;

	}else if(s.compare("--v2")==0){

		opt.lz_variant = lz77_t::v2;

	}else if(s.compare("--p")==0){

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

	}else if(s.compare("--sample-rate")==0){

		if(ptr>=argc){
			cout<<"Missing block size in option --sample-rate" << endl;
			help();
		}

		int sample_rate;
		istringstream ( string(argv[ptr]) ) >> sample_rate;

		if(sample_rate<0){
			cout << "error: sample_rate must be >= 0" << endl;
			help();
		}

		opt.sample_rate=sample_rate;

		ptr++;

	}else if(s.compare("--verbose")==0){

		opt.verbose = true;

	}else if(s.compare("--s")==0){

		if(ptr>=argc){
			cout<<"Missing separator character in option --s" << endl;
			help();
		}

		string sep(argv[ptr]);

		if(sep.length()!=1){
			cout << "Error: separator must be a single character." << endl;
			help();
		}

		opt.sep = sep.at(0);
		ptr++;

	}else if(s.compare("--print-parse")==0){

		print_parse=true;

	}else if(s.compare("--save-parse")==0){

		save_parse=true;

		if(ptr>=argc){
			cout<<"Missing separator and output file name in option --save-parse" << endl;
			help();
		}

		string sep(argv[ptr]);
		sep_out=sep[0];
		ptr++;

		if(ptr>=argc){
			cout<<"Missing output file name in option --save-parse" << endl;
			help();
		}

		out_filename = string(argv[ptr]);
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

	int ptr = 1;

	//parse arguments. Last arg is path
	while(ptr<argc-1)
		opt = parse(opt, ptr,argv, argc);

	//if args are correct, now ptr = argc-1 = path
	if(ptr!=argc-1){

		cout << "Error: missing file path." << endl;
		help();

	}

	opt.mode = lz77_t::file_path;//input string is a file path rather than a text to parse
	opt.prepend_alphabet = false;//don't add the alphabet as prefix

	lz77_t lz77(opt,string(argv[ptr]));

	ofstream out_file;
	if(save_parse)
		out_file.open (out_filename.c_str());

	ulint number_of_phrases=0;
	while(not lz77.end_of_parse()){

		auto token = lz77.get_token();

		if(print_parse){

			cout << "<";

			if(token.start_position_is_defined)
				cout << token.start_position;
			else
				cout << "-";

			cout << ", " << token.phrase << ">" << endl;

		}

		if(save_parse){

			out_file << token.phrase << sep_out;

		}

		number_of_phrases++;

	}

	if(save_parse)
		out_file.close();

	cout << "number of phrases: " << number_of_phrases << endl;

 }

