/*
 * BackwardFileReader.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: nicola
 */

#include "BackwardFileReader.h"

namespace bwtil {

BackwardFileReader::BackwardFileReader(string path){

	this->path=path;

	fp = fopen(path.c_str(), "rb");

	if (fp == NULL){
	  cout << "Error while opening file " << path <<endl;
	  exit(0);
	}

	fseek(fp, 0, SEEK_END);
	n = ftell(fp);

	if (n == 0){
	  cout << "Error: file " << path << " has length 0." << endl;
	  exit(0);
	}

	bufferSize = (ulint)(log2(n+1)*log2(n+1));

	buffer = new symbol[bufferSize];

	/*cout << "bufferSize="<<bufferSize<<endl;
	cout << "txt len="<<n<<endl;*/

	rewind();

}

void BackwardFileReader::rewind(){//go back to EOF

	offset = (n/bufferSize)*bufferSize;

	if(offset==n)
		offset = n - bufferSize;

	ulint size = n-offset;

	fseek ( fp , offset , SEEK_SET );

	if(fread(buffer, sizeof(symbol), size, fp)==0){
		cout << "Error while reading file " << path <<endl;
		exit(0);
	}

	begin_of_file=false;

	ptr_in_buffer = size-1;

	read();//skip newline (at the end of the file)

}

symbol BackwardFileReader::read(){

	symbol s = buffer[ptr_in_buffer];

	if(ptr_in_buffer==0){//read new chunk

		if(offset==0){
			begin_of_file=true;
			return s;
		}

		offset -= bufferSize;

		fseek ( fp , offset , SEEK_SET );

		if(fread(buffer, sizeof(symbol), bufferSize, fp)==0){
			cout << "Error while reading file " << path <<endl;
			exit(0);
		}

		ptr_in_buffer = bufferSize-1;

	}else{
		ptr_in_buffer--;
	}

	return s;

}

} /* namespace compressed_bwt_construction */
