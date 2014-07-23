//============================================================================
// Name        : sFM-index.cpp
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : test class for the succinct FM index
//============================================================================

#include <iostream>

#include "../../data_structures/succinctFMIndex.h"
#include "../../extern/getRSS.c"

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

}

