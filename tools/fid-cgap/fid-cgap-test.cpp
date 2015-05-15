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
#include "../../data_structures/fid_cgap.h"
#include "set"

using namespace bwtil;

 int main(int argc,char** argv) {

#ifdef DEBUG
	 cout << "\n ****** DEBUG MODE ******\n\n";
#endif

	if(argc != 2){
		cout << "*** test fid_cgap structure: converts each run of the input file in a bit run 00...001 and builds the cgap FID on the resulting bitvector. This code serves just for testing purposes***\n";
		cout << "Usage: fid-cgap-test text_file\n";
		exit(0);
	}

	string path = string(argv[1]);
	FileReader text(path);
	ulint length = text.size();

	vector<bool> B;

	uchar last_char = text.get();

	while(not text.eof()){

		uchar c = text.get();

		if(c!=last_char){

			B.push_back(true);
			last_char=c;

		}else{
			B.push_back(false);
		}

	}

	text.close();

	//build FID
	fid_cgap fid(B);

	std::ofstream out("/home/nicola/workspace/BWTIL/FID",std::ofstream::binary);
	fid.serialize(out);
	out.close();

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;

    auto t1 = high_resolution_clock::now();

    cout << "bechmark rank ... " << endl;
    uint last_perc=0;
    uint perc=0;
    ulint nr_trials = 1000000;
    srand(time(NULL));
    for(ulint i=0;i<nr_trials;++i){

    	ulint j = rand()%fid.size();

    	fid.rank(j);
    	perc=(100*i)/nr_trials;
    	if(perc>=last_perc+10){
    		last_perc=perc;
    		cout << " " << perc << "% done ..." << endl;
    	}

    }

    auto t2 = high_resolution_clock::now();
	double total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();

	//print stats
	cout << endl << "microseconds/rank = " << 1000000*(double)total/nr_trials << endl<<endl;

	t1 = high_resolution_clock::now();
    cout << "bechmark access ... " << endl;
    last_perc=0;
    perc=0;
    srand(time(NULL));
    for(ulint i=0;i<nr_trials;++i){

    	ulint j = rand()%fid.size();

    	fid[j];
    	perc=(100*i)/nr_trials;
    	if(perc>=last_perc+10){
    		last_perc=perc;
    		cout << " " << perc << "% done ..." << endl;
    	}

    }

    t2 = high_resolution_clock::now();
	total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();

	//print stats
	cout << endl << "microseconds/access = " << 1000000*(double)total/nr_trials << endl<<endl;

	t1 = high_resolution_clock::now();
    cout << "bechmark gapAt ... " << endl;
    last_perc=0;
    perc=0;
    srand(time(NULL));
    for(ulint i=0;i<nr_trials;++i){

    	ulint j = rand()%fid.number_of_1();

    	fid.gapAt(j);
    	perc=(100*i)/nr_trials;
    	if(perc>=last_perc+10){
    		last_perc=perc;
    		cout << " " << perc << "% done ..." << endl;
    	}

    }

    t2 = high_resolution_clock::now();
	total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();

	//print stats
	cout << endl << "microseconds/gapAt = " << 1000000*(double)total/nr_trials << endl<<endl;

	t1 = high_resolution_clock::now();
    cout << "bechmark select ... " << endl;
    last_perc=0;
    perc=0;
    srand(time(NULL));
    for(ulint i=0;i<nr_trials;++i){

    	ulint j = rand()%fid.number_of_1();

    	fid.select(j);
    	perc=(100*i)/nr_trials;
    	if(perc>=last_perc+10){
    		last_perc=perc;
    		cout << " " << perc << "% done ..." << endl;
    	}

    }

    t2 = high_resolution_clock::now();
	total = duration_cast<duration<double, std::ratio<1>>>(t2 - t1).count();

	//print stats
	cout << endl << "microseconds/select = " << 1000000*(double)total/nr_trials << endl<<endl;









	cout << "Size of FID (Bytes) = " << fid.bytesize() << endl;
	cout << "n (bits set) = " << fid.number_of_1() << endl;
	cout << "u (bitvector length) = " << fid.size() << endl << endl;

	ulint dist_gaps = fid.number_of_distinct_gaps();

	cout << "D bytesize = " << (fid.D_bytesize()) << endl;
	cout << "C bytesize = " << (fid.C_bytesize()) << endl;
	cout << "C addr bytesize = " << (fid.C_addr_bytesize()) << endl;
	cout << "first_el bytesize = " << (fid.first_el_bytesize()) << endl;
	cout << "fid arrays bytesize = " << (fid.fid_arrays_bytesize()) << endl;
	cout << "# of distinct gaps = " << dist_gaps << endl;
	cout << "bits per distinct gap in D = " << ((double)fid.D_bytesize()*8)/dist_gaps << endl<<endl;

	cout << "prefix length = " << fid.get_prefix_length() << endl<<endl;

	double H0 = fid.entropy();
	double bits_per_el = (double)(fid.bytesize()*8)/fid.number_of_1();

	cout << "bits per element = " << bits_per_el << endl;
	cout << "Entropy (bits) = " << H0 << endl;
	cout << "distance from entropy (bits) = " << (bits_per_el-H0) << endl;
	cout << "compression ratio = " << (double)(fid.bytesize()*8)/fid.size() << endl<<endl;

 }

