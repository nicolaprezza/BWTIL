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
#include "../../data_structures/succinct_vector.h"
#include "../../extern/bitvector/include/bitvector.h"
#include "../../data_structures/sparse_bitvector.h"
#include "../../data_structures/dynamic_vector.h"
#include "../../data_structures/DynamicBWT.h"


using namespace bwtil;
using namespace bv;

void bug1(){

	srand(time(NULL));
	bool rand_bit;
	ulint rand_pos;

	ulint N=4033;

	for(uint j=0;j<10000;j++){

		bitvector_t<2048, alloc_on_demand> bv_Btree(N,256);

		for(ulint i=0;i<N;i++){

			rand_bit = rand()%2;
			rand_pos = rand()%(i+1);

			bv_Btree.insert(rand_pos,rand_bit);

		}

		cout << "done."<< endl;

	}

}

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

	 vector<ulint> f;
	 f.push_back(4);
	 f.push_back(1);
	 f.push_back(2);
	 f.push_back(4);

	 dynamic_bwt_t bwt(f,64);

	 bwt.extend(0);
	 bwt.extend(2);
	 bwt.extend(2);
	 bwt.extend(0);
	 bwt.extend(3);
	 bwt.extend(3);
	 bwt.extend(0);
	 bwt.extend(3);
	 bwt.extend(3);
	 bwt.extend(0);
	 bwt.extend(1);

	 for(uint i=0;i<bwt.size();i++)
		 cout << remap(bwt[i]);

	 cout << endl;

	 for(uint i=0;i<bwt.size();i++)
		cout << bwt.locate_right(i) << " ";

	 cout << endl;


 }




