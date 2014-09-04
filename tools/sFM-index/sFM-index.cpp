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

//============================================================================
// Name        : sFM-index.cpp
// Author      : Nicola Prezza
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : test class for the succinct FM index
//============================================================================

#include <iostream>

#include "../../data_structures/succinctFMIndex.h"

using namespace bwtil;
using namespace std;


int main(int argc,char** argv) {

	if(argc != 4 and argc != 3){
		cout << "*** succinct FM-index data structure : a wavelet-tree based uncompressed FM index ***\n";
		cout << "Usage: sFM-index option file [pattern]\n";
		cout << "where:\n";
		cout <<	"- option = build|search. \n";
		cout << "- file = path of the text file (if build mode) or .sfm sFM-index file (if search mode). \n";
		cout << "- pattern = must be specified in search mode. It is the pattern to be searched in the index.\n";
		exit(0);
	}

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;

	int build=0,search=1;

	int mode;

	if(string(argv[1]).compare("build")==0)
		mode=build;
	else if(string(argv[1]).compare("search")==0)
		mode=search;
	else{
		cout << "Unrecognized option "<<argv[1]<<endl;
		exit(0);
	}

	string in(argv[2]);
	string out(in);
	out.append(".sfm");

	string pattern;

	if(mode==search){
		pattern = string(argv[3]);
	}

    auto t1 = high_resolution_clock::now();

	succinctFMIndex SFMI;

	if(mode==build){

		cout << "Building succinct FM-index of file "<< in << endl;
		SFMI = succinctFMIndex(in,true);

		cout << "\nStoring succinct FM-index in "<< out << endl;
		SFMI.saveToFile(out);

		cout << "Done.\n";

	}

	if(mode==search){

		cout << "Loading succinct FM-index from file "<< in <<endl;
		SFMI = succinctFMIndex::loadFromFile(in);
		cout << "Done." << endl;

		cout << "\nSearching pattern \""<< pattern << "\""<<endl;

		vector<ulint> occ = SFMI.getOccurrencies( pattern );

		cout << "The pattern occurs " << occ.size() << " times in the text at the following positions : \n";

		for(uint i=0;i<occ.size();i++)
			cout << occ.at(i) << " ";

		cout << "\n\nDone.\n";

	}

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

