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
		cout << "*** BWT invert ***\n";
		cout << "Given a bwt file, invert it to reconstruct original text file.\n";
		cout << "Usage: bwt-invert bwt_file output_text_file\n";
		cout << "where:\n";
		cout <<	"- bwt_file is a valid bwt of some text file, with a 0x0 byte as terminator character (must appear only once in the bwt!).\n";
		cout <<	"- output_text_file is inverted bwt produced by bwt-inverter (without the 0x0 terminator)\n";
		exit(0);
	}

	FileReader bwt_fr(argv[1]);

	ulint n_bwt = bwt_fr.size();
	ulint n_inv_bwt = n_bwt-1;

	uchar * bwt = new uchar[n_bwt];//with text terminator 0x0

	bwt_fr.read(bwt,n_bwt);
	bwt_fr.close();

	cout << "Indexing the BWT ... " << endl << endl;

	IndexedBWT idxBWT = IndexedBWT(bwt,n_bwt,0,true);

	delete [] bwt;

	uchar * inverted_bwt = new uchar[n_inv_bwt];//without text terminator 0x0

	cout << "\nDone. Inverting the BWT  ... " << endl;

	//invert the bwt

	ulint i=0;//number of steps
	ulint bwt_pos=0;//position in the L column of the BWT.

	int perc, last_perc=-1;

	while(i<n_inv_bwt){

		inverted_bwt[n_inv_bwt-i-1] = idxBWT.at(bwt_pos);
		bwt_pos = idxBWT.LF(bwt_pos);
		i++;

		perc = (i*100)/(n_inv_bwt-1);
		if(perc>last_perc and perc%10==0){

			cout << perc << "% done"<<endl;
			last_perc=perc;

		}

	}

	cout << "\nDone. Saving the inverted BWT to file ... " << endl;

	FILE * fp;
	if ((fp = fopen(argv[2], "wb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file "  << argv[2] <<endl;
		exit(1);
	}

	fwrite(inverted_bwt, sizeof(uchar), n_inv_bwt, fp);
	fclose(fp);

	cout << "Done. Inverted BWT saved in " << argv[2] << endl;

	printRSSstat();

 }



