/*
 * FileReader.h
 *
 *  Created on: Jul 8, 2014
 *      Author: nicola
 */

#ifndef FILEREADER_H_
#define FILEREADER_H_

#include "../common/common.h"

namespace bwtil {

class FileReader {

public:

	FileReader(string path){

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

	uchar get(){

		fs.read(&x,1);
		pos++;
		return (uchar)x;

	}

	void rewind(){

		fs.seekg (0, ios::beg);
		pos=0;

	}

	void read(uchar * buf, ulint n){

		for(ulint i=0;i<n;i++)
			buf[i] = get();

	}

	string toString(){

		string s="";

		while(not eof())
			s += get();

		rewind();

		return s;

	}

	ulint size(){return n;}

	bool eof(){return pos>=n;}

	void close(){fs.close();}

private:

	string path;
	std::ifstream fs;
	ulint n;

	char x;

	ulint pos;

};

} /* namespace bwtil */
#endif /* FILEREADER_H_ */
