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

	FileReader();

	FileReader(string path){

		fs = new ifstream(path.c_str(), ios::binary);

		streampos begin,end;
		begin = fs->tellg();
		fs->seekg (0, ios::end);
		end = fs->tellg();
		fs->seekg (0, ios::beg);

		n=end-begin;
		pos=0;

	}

	uchar get(){

		char x;
		fs->read(&x,1);
		pos++;

		return (uchar)x;

	}

	void rewind(){

		fs->seekg (0, ios::beg);
		pos=0;

	}

	void read(uchar * buf, ulint n){

		for(ulint i=0;i<n;i++)
			buf[i] = get();

	}

	string toString(){

		rewind();
		string s;

		s = string(n,'e');//allocate space for n chars
		fs->read((char *)s.data(),n);

		/*while(not eof())
			s += get();*/

		rewind();

		return s;

	}

	ulint size(){return n;}

	bool eof(){return pos>=n;}

	void close(){fs->close();delete fs;}

private:

	std::ifstream * fs;
	ulint n;

	ulint pos;

};

} /* namespace bwtil */
#endif /* FILEREADER_H_ */
