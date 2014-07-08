/*
 * FileReader.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: nicola
 */

#include "FileReader.h"

namespace bwtil {

FileReader::FileReader(string path){

	this->path=path;

	fs.open (path.c_str(), ios::binary );

	streampos begin,end;
	begin = fs.tellg();
	fs.seekg (0, ios::end);
	end = fs.tellg();
	fs.seekg (0, ios::beg);

	n=end-begin;
	pos=0;

}

uchar FileReader::get(){

	fs.read(&x,1);
	pos++;
	return (uchar)x;

}

void FileReader::rewind(){

	fs.seekg (0, ios::beg);
	pos=0;

}

void FileReader::read(uchar * buf, ulint n){

	for(ulint i=0;i<n;i++)
		buf[i] = get();

}


} /* namespace bwtil */
