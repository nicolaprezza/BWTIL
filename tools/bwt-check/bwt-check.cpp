/*
 * bwt-check.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../data_structures/IndexedBWT.h"

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
		cout << "  bwt_file is the bwt of text_file, with a 0x0 byte as terminator character (must appear only once in the bwt!)\n";
		exit(0);
	}


 }



