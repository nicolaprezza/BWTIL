//============================================================================
// Name        : dB-hash.cpp
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : test class for the dB-hash data structure. Allows to build a dB-hash over a text file and query it with a pattern.
//============================================================================

#include <iostream>

#include "../../data_structures/StaticBitVector.h"
#include "../../data_structures/DBhash.h"
#include "../../data_structures/HashFunction.h"
#include "../../extern/getRSS.h"
#include "../../data_structures/FileReader.h"

using namespace bwtil;
using namespace std;


/*
 * input: path of a text file, pattern length m
 * returns: a dB-hash data structure built on the text. Hash function used is the default (base b=4). word size w used is the optimal w=log_b(m*n), where n=text length
 */
DBhash buildFromFile(string text_path, uint m){

	// 1) read text from file

	FileReader fr = FileReader(text_path);
	string text = fr.toString();
	fr.close();

	// 2) create the hash function

	//HashFunction * h = new HashFunction(n,m,DNA_SEARCH);//hash function for DNA search. Use only if the file is on the alphabet {A,C,G,T,N}
	HashFunction h = HashFunction(m,text_path,true);//general purpose hash function: detect automatically alphabet size

	//build dBhash data structure

	DBhash dBhash = DBhash(text,h,4,true);//offrate=4, verbose=true

	return dBhash;

}

 int main(int argc,char** argv) {

	//debug();

	if(argc != 4){
		cout << "*** dB-hash data structure ***\n";
		cout << "Usage: dB-hash option file [pattern] [pattern_length]\n";
		cout << "where: \n";
		cout <<	"- option = build|search.\n";
		cout <<	"- file = path of the text file (if build mode) or dB-hash .dbh file (if search mode). \n";
		cout << "- pattern_length = In build mode, specify this parameter, which is the pattern length\n";
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

	string in = string(argv[2]);
	string out = string(in);
	out.append(".dbh");

	uint m;

	string pattern;

	if(mode==search){
		pattern = string(argv[3]);
		m = pattern.length();
	}

	if(mode==build)
		m = atoi(argv[3]);

    auto t1 = high_resolution_clock::now();

	DBhash dBhash;

	if(mode==build){


		cout << "Building dB-hash of file "<< in << endl;
		dBhash = buildFromFile(in,m);

		cout << "\nStoring dB-hash in "<< out<<endl;
		dBhash.saveToFile(out);

		cout << "Done.\n";

	}

	if(mode==search){

		cout << "Loading dB-hash from file "<< in <<endl;
		dBhash = DBhash::loadFromFile(in);
		cout << "Done." << endl;

		if(dBhash.patternLength()!=m){
			cout << "Error: structure built with pattern length " << dBhash.patternLength() << ", but now searching a pattern of length " << m << endl;
			exit(1);
		}

		cout << "\nSearching pattern "<< pattern <<endl;

		vector<ulint> occ = dBhash.getOccurrencies( pattern );

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

