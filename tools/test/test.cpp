/*
 * main.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#include "../../common/common.h"
#include "../../algorithms/cw_bwt.h"
#include "../../data_structures/FileReader.h"
#include "../../data_structures/DummyDynamicBitvector.h"
#include "../../extern/bitvector/include/bitvector.h"

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

 int main(int argc,char** argv) {

	string txt = FileReader("/home/nicola/workspace/datasets/pizzachilli/plain/english.10MB").toString();

	auto cwbwt = cw_bwt(txt,cw_bwt::text,true);

	string bwt = cwbwt.toString();

	printRSSstat();

 }




