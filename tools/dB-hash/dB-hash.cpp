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

using namespace bwtil;
using namespace std;


/*
 * input: path of a text file, pattern length m
 * returns: a dB-hash data structure built on the text. Hash function used is the default (base b=4). word size w used is the optimal w=log_b(m*n), where n=text length
 */
DBhash buildFromFile(const char * text_path, uint m){

	// 1) read text from file

	FILE *fp;

	if ((fp = fopen(text_path, "rb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file "  << text_path <<endl;
		exit(1);
	}

	fseek(fp,0,SEEK_END);
	ulint n = ftell(fp);
	fseek(fp,0,SEEK_SET);

	if(n==0){
		VERBOSE_CHANNEL<< "Empty file. "  << text_path <<endl;
		exit(1);
	}else
		n--;//remove terminator char

	unsigned char * text = new unsigned char[n];
	ulint n_bytes = fread(text, sizeof(unsigned char), n, fp);

	if(n_bytes==0){

		VERBOSE_CHANNEL<< "Error while reading file "  << text_path <<endl;
		exit(1);

	}


	fclose(fp);

	// 2) create the hash function

	//HashFunction * h = new HashFunction(n,m,DNA_SEARCH);//hash function for DNA search. Use only if the file is on the alphabet {A,C,G,T,N}
	HashFunction h = HashFunction(m,text_path,true);//general purpose hash function: detect automatically alphabet size

	//build dBhash data structure

	DBhash dBhash = DBhash(text,n,h,4,true);

	return dBhash;

}

void debug(){

	ulint n = 100;

	StaticBitVector * bv = new StaticBitVector(n);
	vector<bool> * bv_correct = new vector<bool>(n);

	srand(time(NULL));

	for(ulint i=0;i<n;i++){

		uint b = rand()%2;

		bv->setBit(i,b);
		bv_correct->at(i)=b;

	}

	bv->computeRanks();

	uint rank=0;

	for(ulint i=0;i<n;i++){

		if(i>0)
			rank+=bv_correct->at(i-1);

		if(rank!=bv->rank1(i)){
			cout << "rank ERROR in position " << i << " : correct=" << rank << ", wrong= "<<bv->rank1(i)<<endl;


			for(uint j=0;j<n;j++)
				cout << bv_correct->at(j);
			cout<<endl;
			for(uint j=0;j<n;j++)
				cout << bv->bitAt(j);
			cout<<endl;


			exit(0);
		}

	}


	cout << "TEST TERMINATED.\n";

	exit(0);

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

	string in(argv[2]);
	string out(in);
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


		cout << "Building dB-hash of file "<< in<<endl;
		dBhash = buildFromFile(in.c_str(),m);

		cout << "\nStoring dB-hash in "<< out<<endl;
		dBhash.saveToFile(out.c_str());

		cout << "Done.\n";

	}

	if(mode==search){

		cout << "Loading dB-hash from file "<< in <<endl;
		dBhash = DBhash::loadFromFile(in.c_str());
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
	double total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();
	cout << "Total time: " << total << "s.\n";

}

