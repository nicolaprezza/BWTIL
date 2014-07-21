/*
 * bwt-check.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../data_structures/IndexedBWT.h"
#include "../../data_structures/FileReader.h"
#include "../../extern/getRSS.c"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 3){
		cout << "*** BWT check ***\n";
		cout << "Given a bwt file and a text file, checks if the former is the valid bwt of the latter\n";
		cout << "Usage: bwt-check bwt_file text_file\n";
		cout << "where:\n";
		cout <<	"- bwt_file is the bwt of text_file, with a 0x0 byte as terminator character (must appear only once in the bwt!).\n";
		cout <<	"- bwt_file is the plain text file whose bwt is supposed to be stored in bwt_file.\n";
		exit(0);
	}

	FileReader bwt_fr(argv[1]);

	ulint n_bwt = bwt_fr.size();
	ulint n_inv_bwt = n_bwt-1;

	uchar * bwt = new uchar[n_bwt];//with text terminator 0x0
	uchar * inverted_bwt = new uchar[n_inv_bwt];//without text terminator 0x0

	bwt_fr.read(bwt,n_bwt);
	bwt_fr.close();

	cout << "Indexing the BWT ... \n\n";

	IndexedBWT idxBWT = IndexedBWT(bwt,n_bwt,0,true);

	delete [] bwt;

	cout << "\nDone. Inverting the BWT ... " << endl;

	//invert the bwt

	ulint i=0;//number of steps
	ulint bwt_pos=0;//position in the L column of the BWT.

	while(i<n_inv_bwt){

		inverted_bwt[n_inv_bwt-i-1] = idxBWT.at(bwt_pos);
		bwt_pos = idxBWT.LF(bwt_pos);
		i++;

	}

	cout << "Done.\n";

	FileReader text_fr(argv[2]);
	ulint n_text = text_fr.size();

	if(n_text!=n_inv_bwt){

		cout << "\nError: the bwt file and the text input file do not have same length (excluding bwt terminator)\n";
		cout << "text length = " << n_text << ", bwt length (without terminator) = " << n_inv_bwt << endl;
		cout << argv[1] << " " << "is not a valid BWT of " << argv[2] << endl;
		exit(0);

	}

	i=0;

	while(i<n_text){

		if(inverted_bwt[i] != text_fr.get()){

			cout << "\nError: text file and inverted bwt do not match.\n";
			cout << argv[1] << " " << "is not a valid BWT of " << argv[2] << endl;
			exit(0);

		}

		i++;

	}

	cout << "\nSUCCESS!\n";
	cout << argv[1] << " " << "is the BWT of " << argv[2] << endl;

	printRSSstat();

 }



