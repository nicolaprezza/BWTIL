/*
 * fid_cgap.h
 *
 *  Created on: May 8, 2015
 *      Author: nicola
 *
 *  fully-indexable dictionary using Huffman-compressed gaps
 */

#ifndef FID_CGAP_H_
#define FID_CGAP_H_

#include "cgap_dictionary.h"
#include "packed_view.h"
#include "../common/common.h"

namespace bwtil{

class fid_cgap{

public:

	fid_cgap(){}

	fid_cgap(vector<bool> &B){

		build(B);

	}

	ulint size(){
		return u;
	}

	ulint number_of_1(){
		return n;
	}

	/*
	 * \param i<=size() position in the bitvector
	 * \return number of '1' before position i excluded
	 */
	ulint rank(ulint i){

		return 0;

	}

	/*
	 * \param i<number_of_1()
	 * \return position of the i-th bit set. i starts from 0 (first bit set has i=0)
	 */
	ulint select(ulint i){

		return 0;

	}

	/*
	 * returns i-th bit of the underlying bitvector
	 */
	bool operator[](ulint i){

		return 0;

	}

	/*
	 * retrieve length of the i-th gap (i>=0). gap length includes the leading 1
	 * \param i<number_of_1()
	 *
	 */
	ulint gapAt(ulint i){

		return 0;

	}

	/*
	 * \return bit size of the data structure
	 */
	ulint bytesize(){

		return 0;

	}

	/* serialize the structure to the ostream
	 * \param out	 the ostream
	 */
	ulint serialize(std::ostream& out){

		return 0;

	}

	/* load the structure from the istream
	 * \param in the istream
	 */
	void load(std::istream& in) {

	}

private:

	void build(vector<bool> &B){

		assert(B.size()>0);

		{
			auto gaps = cgap_dictionary::bitvector_to_gaps(B);
			D = cgap_dictionary::build_dictionary(gaps);
		}

		W = sizeof(ulint)*8;

		u = 0;
		n = 0;
		for(auto b : B){

			u++;
			n += (b?1:0);

		}

		//compute block size (at most u)
		v = (u*intlog2(u)*intlog2(u))/n;
		if(v>u) v=u;

		t = intlog2(u)*intlog2(u);

		number_of_blocks = u/v + (u%v!=0);

		assert(number_of_blocks>0 and number_of_blocks<=u);

		//V rank in i-1
		ulint last_V_rank = 0;

		for(ulint i = 0;i<number_of_blocks;++i){

			ulint l = i*v;
			ulint r = (i+1)*v;

			assert(l<u);

			if(r>u) r = u;

			vector<bool> subB = {B.begin()+l,B.begin()+r};

			V_rank.push_back(last_V_rank);

			if(not empty(subB)){

				V_select.push_back(i);
				last_V_rank++;

				BSDs.push_back({subB,&D});

			}

		}



	}

	bool empty(vector<bool> &B){

		for(auto b : B)
			if(b) return false;

		return true;

	}

	ulint W=0;

	//number of bits/number of 1s
	ulint u=0,n=0;

	//block size
	ulint v=0;

	//sample select every t bits set
	ulint t=0;

	ulint number_of_blocks=0;

	//Dictionary: supports encoding/decoding of gap codes (Huffman)
	cgap_dictionary D;

	//naive (but ultrafast) constant-time rank-select bitvector taking 2ulog u bits
	vector<ulint> V_rank;
	vector<ulint> V_select;

	//sampled rank results
	vector<ulint> R;

	//one BSD per nonempty block
	vector<bsd_cgap> BSDs;

	//sampled select on blocks
	vector<ulint> SEL;

};

}

#endif /* FID_CGAP_H_ */
