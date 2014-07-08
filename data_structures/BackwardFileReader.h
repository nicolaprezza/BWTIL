/*
 * BackwardFileReader.h
 *
 *  Created on: Jun 24, 2014
 *      Author: nicola
 *
 *      Description: given a text file path, reads it backwards.
 *      The backward read of the file is implemented buffering chunks of size n/(log^2(n)) from the end
 *      In this way, only log^2(n) calls to fseek are necessary and the total RAM occupancy is only n/(log^2(n))
 *
 *
 */

#ifndef BACKWARDFILEREADER_H_
#define BACKWARDFILEREADER_H_

#include "../common/common.h"

namespace bwtil {

class BackwardFileReader {

public:

	BackwardFileReader(){};

	BackwardFileReader(string path);

	bool BeginOfFile(){return begin_of_file;};//no more symbols to be read

	symbol read();//returns next symbol

	void rewind();//go back to EOF

	void close(){fclose(fp);delete [] buffer;};//close file

	ulint length(){return n;};

private:

	ulint n;
	ulint bufferSize;//n/(log^2(n))

	bool begin_of_file;//begin of file reached

	string path;

	symbol * buffer;

	ulint ptr_in_buffer;//pointer to the current position in buffer
	uint offset;//offset (in the file) of the byte after the next chunk to be read

	FILE * fp;//pointer to file.

};

} /* namespace compressed_bwt_construction */
#endif /* BACKWARDFILEREADER_H_ */
