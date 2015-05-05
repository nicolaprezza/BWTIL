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
#include "set"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 2 and argc != 3){
		cout << "*** Count equal-letter runs in a text file ***\n";
		cout << "Usage: count-runs [options] text_file\n";
		cout << "  -e compute zero-order entropy of the runs\n";
		exit(0);
	}


	ulint R=0;//number of runs

	string text_s;
	bool entropy=false;
	if(argc==2)
		text_s = string(argv[1]);
	else{
		if(string(argv[1]).compare("-e")!=0){
			cout << "Error: unrecognized option " << argv[1] << endl;
			exit(0);
		}
		entropy=true;
		text_s = string(argv[2]);
	}

	FileReader text(text_s);
	ulint length = text.size();

	uchar last_char = text.get();
	R++;


	//run lengths and their frequency
	auto comp = [](pair<ulint,ulint> x, pair<ulint,ulint> y){ return x.first < y.first; };
	std::set<pair<ulint,ulint> ,decltype(comp)> runs_and_freq(comp);

	ulint r_len=1;

	while(not text.eof()){

		uchar c = text.get();

		if(c!=last_char){

			R++;
			last_char=c;

			if(entropy){
				//if run is already present
				if(runs_and_freq.find({r_len,0})!=runs_and_freq.end()){

					auto it = runs_and_freq.find({r_len,0});
					ulint new_freq = it->second+1;
					runs_and_freq.erase(it);
					runs_and_freq.insert({r_len,new_freq});

				}else{
					runs_and_freq.insert({r_len,1});
				}

				r_len=1;
			}

		}else{
			if(entropy)
				r_len++;
		}

	}

	cout << "Number of equal-letter runs = " << R << endl;

	//compute entropy of the runs
	if(entropy){

		double H0=0;

		//for(auto p : runs_and_freq)
			//cout << "run = " << p.first << " freq = " << p.second << endl;

		for(auto p : runs_and_freq)
			H0 += (double)p.second * log2((double)R/(double)p.second);

		H0 = H0/(double)R;

		cout << "Entropy of the runs = " << H0 << " bits" << endl;
		cout << "R*H0 = " << H0*(double)R << " bits" << endl;
		cout << "number of distinct run lengths = " << runs_and_freq.size() << endl;


	}

	text.close();


 }

