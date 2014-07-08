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

	FileReader(string path);
	ulint size(){return n;}

	uchar get();

	bool eof(){return pos>=n;}

	void rewind();

	void read(uchar * buf, ulint n);

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
