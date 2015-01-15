/*
 *  This file is part of BWTIL.
 *  Copyright (c) by
 *  Nicola Prezza <nicolapr@gmail.com>
 *
 *   BWTIL is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   BWTIL is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details (<http://www.gnu.org/licenses/>).
 */

#include "../../data_structures/FileReader.h"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 2){
		cout << "*** Count equal-letter runs in a text file ***\n";
		cout << "Usage: count-runs text_file\n";
		exit(0);
	}


	ulint R=0;//number of runs

	FileReader text(argv[1]);
	ulint length = text.size();

	uchar last_char = text.get();
	R++;

	while(not text.eof()){

		uchar c = text.get();

		if(c!=last_char){

			R++;
			last_char=c;

		}

	}

	text.close();

	cout << "Number of equal-letter runs = " << R << endl;

 }

