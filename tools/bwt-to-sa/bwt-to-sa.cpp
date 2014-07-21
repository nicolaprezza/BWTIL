/*
 * bwt-to-sa.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../data_structures/IndexedBWT.h"
#include "../../data_structures/FileReader.h"
#include "../../extern/getRSS.c"
#include "../../common/common.h"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 3 and argc != 4){
		cout << "*** BWT to Suffix Array converter ***\n";
		cout << "Given a bwt file, the Suffix array is induced and stored to file.\n";
		cout << "Format of ouput file is one unsigned long int for each text position. Size of the output file is therefore 8n Bytes.\n";
		cout << "Usage: bwt-to-sa bwt_file output_sa_file [bufsize]\n";
		cout << "where:\n";
		cout <<	"- bwt_file must be a valid bwt of some text file, with a 0x0 byte as terminator character (must appear only once in the bwt!)\n";
		cout <<	"- output_sa_file is the output SA file.\n";
		cout <<	"- bufsize (optional) is the buffer size for SA addresses. The BWT is traversed n/bufsize times, each time filling the buffer with bufsize addresses. Default: n/log n\n";
		cout << "Space in RAM is  O(n * log sigma + bufsize*log n) bits ( O(n * log sigma) with default bufsize )\n";
		exit(0);
	}

	FileReader bwt_fr(argv[1]);

	ulint n_bwt = bwt_fr.size();
	uchar * bwt = new uchar[n_bwt];//with text terminator 0x0

	bwt_fr.read(bwt,n_bwt);
	bwt_fr.close();

	cout << "Indexing the BWT ... " << endl << endl;

	IndexedBWT idxBWT = IndexedBWT(bwt,n_bwt,0,true);
	delete [] bwt;

	cout << "\nDone.";

	if(argc==3){//auto bufsize

		idxBWT.storeSuffixArrayToFile(argv[2]);

	}else{// bufsize provided

		idxBWT.storeSuffixArrayToFile(argv[2],atoi(argv[3]));

	}

	cout << "\nDone. Suffix array stored in " << argv[2] << endl;

	printRSSstat();

 }



