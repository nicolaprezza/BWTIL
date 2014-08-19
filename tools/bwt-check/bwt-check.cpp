/*
 * bwt-check.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../data_structures/IndexedBWT.h"
#include "../../data_structures/FileReader.h"
#include "../../extern/getRSS.h"

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
		cout <<	"- text_file is the plain text file whose bwt is supposed to be stored in bwt_file.\n";
		exit(0);
	}

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;

    auto t1 = high_resolution_clock::now();

    IndexedBWT idxBWT;
    ulint n_inv_bwt=0;

    {

		FileReader fr(argv[1]);
		string bwt = fr.toString();
		fr.close();

		n_inv_bwt = bwt.length()-1;

		cout << "BWT length =  " << bwt.length() << endl;
		cout << "Indexing the BWT ... " << endl << endl;

		idxBWT = IndexedBWT(bwt,0,true);

    }

	cout << "\nDone. Inverting the BWT ... " << endl;

	string inverted_bwt = string(n_inv_bwt,'e');//without text terminator 0x0

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

		if((uchar)inverted_bwt[i] != text_fr.get()){

			cout << "\nError: text file and inverted bwt do not match.\n";
			cout << argv[1] << " " << "is not a valid BWT of " << argv[2] << endl;
			exit(0);

		}

		i++;

	}

	cout << "\nSUCCESS!\n";
	cout << argv[1] << " " << "is the BWT of " << argv[2] << endl;

	printRSSstat();

	auto t2 = high_resolution_clock::now();
	double total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();
	cout << "Total time: " << total << "s.\n";

 }



