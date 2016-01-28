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

#include "../../common/common.h"
#include "../../algorithms/cw_bwt.h"
#include "../../data_structures/FileReader.h"
#include "../../data_structures/DummyDynamicBitvector.h"
#include "../../data_structures/sparse_vector.h"
#include "../../data_structures/sparse_bitvector.h"
#include "../../data_structures/succinct_vector.h"
#include "../../data_structures/succinct_bitvector.h"
#include "../../extern/bitvector/include/bitvector.h"
#include "../../data_structures/sparse_bitvector.h"
#include "../../data_structures/dynamic_vector.h"
#include "../../data_structures/DynamicBWT.h"
#include "../../data_structures/cgap_dictionary.h"
#include "../../data_structures/bsd_cgap.h"
#include "../../data_structures/fid_cgap.h"

#include "bitview.h"
#include <vector>


using namespace bwtil;
using namespace bv;



void test1(){

	srand(time(NULL));
	bool rand_bit;
	ulint rand_pos;

	 for(ulint N=1000;N<100000;N++){

		cout << "N = " << N << endl;

		bitvector_t<2048, alloc_on_demand> bv_Btree(N,256);

		cout << "Inserting bits ... "<< flush;

		for(ulint i=0;i<N;i++){

			rand_bit = rand()%2;
			rand_pos = rand()%(i+1);

			bv_Btree.insert(rand_pos,rand_bit);

		}

		cout << "done."<< endl;

		cout << "Checking rank1 correctness ... "<< flush;

		ulint rank=0;

		for(ulint i=0;i<N;i++){

			if(rank != bv_Btree.rank(i,1)){

				cout << "ERROR in rank: \n";

				cout << "bv_Btree.rank("<<i-1<<",1) = " << bv_Btree.rank(i-1,1)<<endl;
				cout << "bv_Btree.rank("<<i<<",1) = " << bv_Btree.rank(i,1)<<endl;

				cout << "correct rank computed on-the fly = " << rank <<endl;

				exit(1);

			}

			rank += bv_Btree.access(i);

		}

		cout << "ok. " <<  endl;

	 }

}


void test2(){

	srand(time(NULL));
	bool rand_bit;
	ulint rand_pos;

	 for(ulint N=1000;N<100000;N++){

		cout << "N = " << N << endl;

		DummyDynamicBitvector bv_naive(N);
		bitvector_t<2048, alloc_on_demand> bv_Btree(N,256);

		cout << "Inserting bits ... "<< flush;

		for(ulint i=0;i<N;i++){

			rand_bit = rand()%2;
			rand_pos = rand()%(i+1);

			bv_naive.insert(rand_pos,rand_bit);
			bv_Btree.insert(rand_pos,rand_bit);

		}

		cout << "done."<< endl;

		cout << "d = " << bv_Btree.info().degree << endl;
		cout << "b = " << bv_Btree.info().buffer << endl;
		cout << "height = " << bv_Btree.info().height << endl <<endl;

		cout << "Checking content correctness ... "<< flush;

		for(ulint i=0;i<N;i++){

			if(bv_naive.access(i) != bv_Btree.access(i)){

				cout << "ERROR: naive bv and Btree bv do not coincide in content \n";
				exit(1);

			}

		}

		cout << "ok. " <<  endl;

		cout << "Checking rank1 correctness ... "<< flush;

		for(ulint i=0;i<N;i++){

			if(bv_naive.rank(i,1) != bv_Btree.rank(i,1)){

				cout << "ERROR: naive bv and Btree bv do not coincide in rank1 \n";

				cout << "bv_naive.rank(1,"<<i<<") = " << bv_naive.rank(i,1)<<endl;
				cout << "bv_Btree.rank(1,"<<i<<") = " << bv_Btree.rank(i,1)<<endl;

				exit(1);

			}

		}

		cout << "ok. " <<  endl;

	 }


}


void test_sparse_vector(){
	 bool tempBool[] = { true, false, false, true, false, true, true };
	 std::vector<bool> bv ( tempBool, tempBool + sizeof(tempBool) / sizeof(bool) );

	 sparse_vector<int> sv(bv);

	 sv[3] = 9;
	 sv[3]++;

	 sv[1]++;

	 sv[0]++;

	 for(uint i=0;i<bv.size();i++)
		 cout << sv[i] << " ";

	 cout << endl;
}

void test_dynamic_vector(){
	 dynamic_vector_t dv(100,16);

	 dv.insert(0,5);
	 dv.insert(0,3);
	 dv.insert(0,2);
	 dv.insert(0,8);
	 dv.insert(1,88);
	 dv.insert(4,123);


	 for(uint i=0;i<dv.size();i++)
		 cout << dv[i] << " " ;

	 cout << endl;
}

char remap(symbol s){

	switch(s){
		case 0: return 'i';
		case 1: return 'm';
		case 2: return 'p';
		case 3: return 's';
	}

	return 'X';

}


 int main(int argc,char** argv) {

	 {
		vector<uchar> a(50000000);
	 }


		printRSSstat();

		exit(0);

/*	 uint w=64;
	 ulint n=1000000;
	 ulint max = 0;

	 if(w<64)
		 max = ulint(1)<<w;

	 packed_view<vector> pv(w,n);

	 srand(time(NULL));

	 for(ulint i=0;i<n;++i)
		 pv[i] = (w<64?rand()%max:rand());

	 std::sort(pv.begin(),pv.end());

	 for(ulint i=1;i<n;++i)
		 if(pv[i]<pv[i-1]){
			 cout << "something's wrong!"<<endl;
			 exit(0);
		 }

	 cout << "OK!" <<endl;*/

/*	 ulint c = 12;

	 bitview<vector> bvv(c);

	 ulint i = 4;
	 ulint j = 7;
	 ulint l = j-i;

	 ulint i1 = (c-i)-l;
	 ulint j1 = c-i;

	 cout << "i1 = " << i1 << ", j1 = " << j1 << endl;

	 bvv(i1,j1) = 6;

	 cout << bvv.get(i1,j1) << endl;

	 i = 4;
	 j = 9;
	 l = j-i;

	 i1 = (c-i)-l;
	 j1 = c-i;

	 cout << bvv.get(i1,j1) << endl;

	 exit(0);*/

/*	 vector<bool> B = {1,0,1,0,1,0,0,0,1,1,0,1,1,1,1,1,1,0,1,1};

	 bool last = B[B.size()-1];
	 auto gaps = cgap_dictionary::bitvector_to_gaps(B);
	 auto D = cgap_dictionary::build_dictionary(gaps);
	 bsd_cgap bsd(gaps,last,&D);

	 cout << (bsd[11]==B[11]) << endl;*/


	 ulint u = 100000;
	 ulint trials=100;

	 double p=0.001;

	 vector<bool> B(u,false);

	 srand(time(NULL));

	 uint last_perc=0;

	 cout << "start!" << endl;

	 for(ulint j=0;j<trials;j++){

		 uint perc = (j*100)/trials;
		 if(perc-last_perc>0){
			 cout << perc << "% done ..." <<endl;
			 last_perc=perc;
		 }

		 for(ulint i=0;i<B.size();++i){

			 ulint max = 99999999;
			 double x = (double)(rand()%max)/(double)max;

			 B[i] = x<p;
		 }

		 //for(auto b:B)
		//	 cout << b;
		 //cout << endl;

		 {
			 fid_cgap fid(B);
			// cout << "saving ... "<<flush;
			 std::ofstream out("/home/nicola/workspace/BWTIL/FID",std::ofstream::binary);
			 fid.serialize(out);
			 out.close();
			// cout << "ok! "<<endl;
		 }

		//cout << "loading ... "<<flush;
		std::ifstream in("/home/nicola/workspace/BWTIL/FID",std::ifstream::binary);
		fid_cgap fid;
		fid.load(in);
		in.close();

/*
		cout << "bits per element = " << (double)(fid.bytesize()*8)/fid.number_of_1() << endl;
		cout << "bits per element C = " << (double)(fid.C_bytesize()*8)/fid.number_of_1() << endl;
		cout << "bits per element D = " << (double)(fid.D_bytesize()*8)/fid.number_of_1() << endl;
		cout << "Entropy = " << fid.entropy() << endl;
		cout << "ratio = " << (double)(fid.bytesize()*8)/B.size() << endl<<endl;
*/

		auto gaps = cgap_dictionary::bitvector_to_gaps(B);

		 sparse_bitvector<> sbv(B);

		 for(uint i=0;i<fid.number_of_1();++i){

			 if(fid.gapAt(i)!=gaps[i]){
				 cout << "ERROR in gapAt"<<endl;
				 exit(0);
			 }

		 }

		 for(uint i=0;i<fid.number_of_1();++i){

			 if(fid.select(i)!=sbv.select(i)){
				 cout << "ERROR in select"<<endl;
				 exit(0);
			 }

		 }

		 for(uint i=0;i<fid.size();++i){

			//cout << i << " -> " << bsd.rank(i) << " / " <<  sbv.rank(i)<<endl;

			 fid[i];

			if(fid.rank(i)!=sbv.rank(i)){
				 cout << "ERROR in rank"<<endl;
				 exit(0);
			}

		 }

		 for(uint i=0;i<fid.size();++i){

			//cout << i << " -> " << bsd.rank(i) << " / " <<  sbv.rank(i)<<endl;

			if(fid[i]!=B[i]){
				 cout << "ERROR in access " << i <<endl;
				 exit(0);
			}

		 }

	 }

	 cout << "OK!" << endl;


 }




