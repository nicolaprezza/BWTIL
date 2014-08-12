/*
 * main.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 */

#include "../../data_structures/DummyDynamicBitvector.h"
#include "../../extern/bitvector/include/bitvector.h"

using namespace bwtil;
using namespace bv;

 int main(int argc,char** argv) {

	 bitvector_t<2048, alloc_on_demand> aa(5,256);

	 exit(0);

	ulint N = 100000;

	DummyDynamicBitvector bv_naive(N);
	bitvector_t<2048, alloc_on_demand> bv_Btree(N,256);

	srand(time(NULL));

	bool rand_bit;
	ulint rand_pos;

	cout << "Inserting bits ... "<< flush;

	for(ulint i=0;i<N;i++){

		rand_bit = rand()%2;
		rand_pos = rand()%(i+1);

		bv_naive.insert(rand_bit,rand_pos);
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

		if(bv_naive.rank(1,i) != bv_Btree.rank(i,1)){

			cout << "ERROR: naive bv and Btree bv do not coincide in rank1 \n";
			exit(1);

		}

	}

	cout << "ok. " <<  endl;

	cout << "Checking rank0 correctness ... "<< flush;

	for(ulint i=0;i<N;i++){

		if(bv_naive.rank(0,i) != bv_Btree.rank(i,0)){

			cout << "ERROR: naive bv and Btree bv do not coincide in rank0 \n";
			exit(1);

		}

	}

	cout << "ok. " <<  endl;

	cout << "Success! " << endl;

 }



