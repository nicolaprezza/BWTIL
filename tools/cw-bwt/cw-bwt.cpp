/*
 * main.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#include "../../algorithms/cw_bwt.h"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 3 and argc != 4){
		cout << "*** context-wise BWT construction in compressed space ***\n";
		cout << "*** WARNING: the tool is still under development and at the moment inefficient data structures are used. Use the tool only on small files (max 10MB) ***\n";
		cout << "Usage: cw-bwt text_file bwt_file [k]\n";
		cout << "where:\n";
		cout << "- text_file is the input text file. Input file must not contain a 0x0 byte since the algorithm uses it as text terminator.\n";
		cout << "- bwt_file is the output bwt file. This output file will contain a 0x0 terminator and thus will be 1 byte longer than the input file.\n";
		cout << "- k (automatic if not specified) is the entropy order.\n";
		cout << "WARNING: sigma^k space will be allocated, where sigma is the alphabet size. If you specify k, choose it carefully!\n";
		cout << "For more informations, read the file README.\n";
		exit(0);
	}

	cw_bwt cwbwt;

	if(argc==3)//no k
		cwbwt = cw_bwt(argv[1],cw_bwt::path);

	if(argc==4)
		cwbwt = cw_bwt(argv[1],cw_bwt::path,atoi(argv[3]),true);

	/*
	 * If, instead, you want to compute the bwt of a string object, build cw_bwt with
	 *
	 * cw_bwt(your_string,cw_bwt::text) // optimal k autodetected
	 *
	 * or
	 *
	 * cw_bwt(your_string,cw_bwt::text, your_k_value,true) // you choose k (faster since k has not to be autodetected)
	 *
	 * However, this requires more space in RAM since the input text string is kept in memory together with the structures of cwbwt
	 *
	 */

	cout << "\nSaving BWT in " << argv[2] << endl;
	cwbwt.toFile(argv[2]);
	cout << "Done. " << endl;

	/*
	 *
	 * If, instead, you want a string object containing the bwt, call
	 *
	 * string bwt = cwbwt.toString();
	 *
	 * However, this requires more space in RAM since the string bwt is kept in memory together with the structures of cwbwt
	 * WARNING: if you directly print cwbwt.toString(), you won't see the terminator character since it is a 0x0 byte.
	 *
	 */

 }



