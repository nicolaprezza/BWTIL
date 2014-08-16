/*
 * bwt-to-sa.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: nicola
 */

#include "../../data_structures/IndexedBWT.h"
#include "../../data_structures/FileReader.h"
#include "../../extern/getRSS.h"
#include "../../common/common.h"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 3 and argc != 4){
		cout << "*** BWT to Suffix Array converter ***\n";
		cout << "Given a bwt file, builds the Suffix array and stores it directly to disk.\n";
		cout << "Format of ouput file is one unsigned long int for each text position. Size of the output file is therefore 8n Bytes.\n";
		cout << "Usage: bwt-to-sa bwt_file output_sa_file [offset]\n";
		cout << "where:\n";
		cout <<	"- bwt_file must be a valid bwt of some text file, with a 0x0 byte as terminator character (must appear only once in the bwt!)\n";
		cout <<	"- output_sa_file is the output SA file.\n";
		cout <<	"- offset (optional). Store explicitly one SA pointer every offset positions of the text. Default: log n/log sigma\n";
		exit(0);
	}

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;

    auto t1 = high_resolution_clock::now();

	FileReader bwt_fr(argv[1]);

	ulint n_bwt = bwt_fr.size();
	uchar * bwt = new uchar[n_bwt];//with text terminator 0x0

	bwt_fr.read(bwt,n_bwt);
	bwt_fr.close();

	cout << "Indexing the BWT ... " << endl << endl;

	IndexedBWT idxBWT;

	if(argc==3){//auto bufsize

		uint offset = log2(n_bwt)/8;
		if(offset==0)
			offset=1;

		idxBWT = IndexedBWT(bwt,n_bwt,offset,true);

	}else{// bufsize provided

		if(atoi(argv[3])<=0){
			cout << "Error: offset must be > 0.\n";
			exit(1);
		}

		idxBWT = IndexedBWT(bwt,n_bwt,atoi(argv[3]),true);

	}

	delete [] bwt;

	cout << "\nDone.\n\nStoring suffix array to file ... " << endl <<endl;

	FILE *fp;

	if ((fp = fopen(argv[2], "wb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file "  << argv[2] <<endl;
		exit(1);
	}

	int perc,last_perc=-1;
	for(ulint i=1;i<n_bwt;i++){

		ulint idx = idxBWT.convertToTextCoordinate(i);

		if(idx>=n_bwt-1){

			cout << "Error: SA address greater than text length. " << idx << ">=" << n_bwt-1 << endl;
			cout << "Debug needed.\n";
			exit(1);

		}

		fwrite(&idx, sizeof(ulint), 1, fp);

		perc=(100*(i+1))/n_bwt;

		if(perc>last_perc and perc%10==0){

			cout << perc << "% done"<<endl;
			last_perc=perc;

		}

	}

	cout << "\nDone. Suffix array stored in " << argv[2] << endl;

	fclose(fp);

	printRSSstat();

	auto t2 = high_resolution_clock::now();
	double total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();
	cout << "Total time: " << total << "s.\n";


 }



