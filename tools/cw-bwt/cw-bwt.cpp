/*
 * main.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#include "../../data_structures/CwBWT.h"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	/* vector<ulint> freq;

	 freq.push_back(22);
	 freq.push_back(0);
	 freq.push_back(21);
	 freq.push_back(5);
	 freq.push_back(3);
	 freq.push_back(0);
	 freq.push_back(9);

	 ulint n=0;
	 for(uint i=0;i<freq.size();i++)
		 n+= freq.at(i);

	 DynamicString * dyns = DynamicString::newDynamicString(n,freq);

	 for(uint k=0;k<freq.size();k++){

		 for(uint i=0;i<freq.at(k);i++)
			 dyns->insert(k,0);

	 }

	 cout << dyns->toString()<<endl;*/

	/* uint sigma = 254;
	 CumulativeCounters cc(sigma,1000);

	cout << cc.toString() << endl;

	cc.increment(2);
	cout << cc.toString() << endl;

	cout << cc.toString() << endl;*/


	if(argc != 3 and argc != 4){
		cout << "*** context-wise BWT construction in compressed space ***\n";
		cout << "Usage: cw-bwt text_path bwt_path [k]\n";
		cout << "where:\n";
		cout << "  text_path is the input text path. Input file must not contain a 0x0 byte since the algorithm uses it as text terminator.\n";
		cout << "  bwt_path is the output bwt path. This output file will contain a 0x0 terminator and thus will be 1 byte longer than the input file.\n";
		cout << "  k (automatic if not specified) is the entropy order.\n";
		cout << "WARNING: sigma^k space will be allocated, where sigma is the alphabet size. If you specify k, choose it carefully!\n";
		cout << "For more informations, read the file README.\n";
		exit(0);
	}

	CwBWT bwt;

	if(argc==3)//no k
		bwt = CwBWT(argv[1]);

	if(argc==4)
		bwt = CwBWT(argv[1],atoi(argv[3]),true);

	bwt.toFile(argv[2]);

 }



