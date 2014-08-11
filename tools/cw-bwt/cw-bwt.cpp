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

	if(argc != 3 and argc != 4){
		cout << "*** context-wise BWT construction in compressed space ***\n";
		cout << "*** WARNING: the tool is still under development and at the moment inefficient data structures are used. Use the tool only on small files (max 10MB) ***\n";
		cout << "Usage: cw-bwt text_file bwt_file [k]\n";
		cout << "where:\n";
		cout << "- text_file is the input text file. Input file must not contain a 0x0 byte since the algorithm uses it as text terminator.\n";
		cout << "- bwt_file is the output bwt file. This output file will contain a 0x0 terminator and thus will be 1 byte longer than the input file.\n";
		cout << "- k (automatic if not specified) is the entropy order.\n";
		cout << "WARNING: sigma^k space will be allocated, where sigma is the alphabet size. If you specify k, choose it carefully!\n";
		cout << "For more informations, read the file README.\n";
		exit(0);
	}

	CwBWT bwt;

	if(argc==3)//no k
		bwt = CwBWT(argv[1]);

	if(argc==4)
		bwt = CwBWT(argv[1],atoi(argv[3]),true);

	cout << "\nSaving BWT in " << argv[2] << endl;
	//bwt.toFile(argv[2]);
	cout << "Done. " << endl;

 }



