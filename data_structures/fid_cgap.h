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

		ulint varsize = 	sizeof(W) +
							sizeof(u) +
							sizeof(n) +
							sizeof(v) +
							sizeof(t) +
							sizeof(number_of_v_blocks) +
							sizeof(number_of_t_blocks) +
							sizeof(u);

		ulint bsd_size = sizeof(BSDs);

		for(auto bsd : BSDs)
			bsd_size += bsd.bytesize();

		return 	varsize +
				D.bytesize() +
				sizeof(V_rank) +
				V_rank.size()*sizeof(ulint) +
				sizeof(V_select) +
				V_select.size()*sizeof(ulint) +
				sizeof(R) +
				R.size()*sizeof(ulint) +
				sizeof(SEL) +
				SEL.size()*sizeof(ulint) +
				bsd_size;

	}

	/* serialize the structure to the ostream
	 * \param out	 the ostream
	 */
	ulint serialize(std::ostream& out){

		out.write((char *)&W, sizeof(ulint));
		out.write((char *)&u, sizeof(ulint));
		out.write((char *)&n, sizeof(ulint));
		out.write((char *)&v, sizeof(ulint));
		out.write((char *)&t, sizeof(ulint));
		out.write((char *)&number_of_v_blocks, sizeof(ulint));
		out.write((char *)&number_of_t_blocks, sizeof(ulint));

		ulint v_sel_size = V_select.size();
		out.write((char *)&v_sel_size, sizeof(ulint));

		ulint w_bytes = 8*sizeof(ulint);

		D.serialize(out);

		out.write((char *)V_rank.data(), number_of_v_blocks*sizeof(ulint));
		w_bytes += number_of_v_blocks*sizeof(ulint);

		out.write((char *)V_select.data(), v_sel_size*sizeof(ulint));
		w_bytes += v_sel_size*sizeof(ulint);

		out.write((char *)R.data(), number_of_v_blocks*sizeof(ulint));
		w_bytes += number_of_v_blocks*sizeof(ulint);

		for(auto bsd : BSDs)
			w_bytes += bsd.serialize(out);

		out.write((char *)SEL.data(), number_of_t_blocks*sizeof(ulint));
		w_bytes += number_of_t_blocks*sizeof(ulint);

		return w_bytes;

	}

	/* load the structure from the istream
	 * \param in the istream
	 */
	void load(std::istream& in) {

		in.read((char *)&W, sizeof(ulint));
		in.read((char *)&u, sizeof(ulint));
		in.read((char *)&n, sizeof(ulint));
		in.read((char *)&v, sizeof(ulint));
		in.read((char *)&t, sizeof(ulint));
		in.read((char *)&number_of_v_blocks, sizeof(ulint));
		in.read((char *)&number_of_t_blocks, sizeof(ulint));

		ulint v_sel_size;
		in.read((char *)&v_sel_size, sizeof(ulint));

		D.load(in);

		V_rank = {number_of_v_blocks};
		in.read((char *)V_rank.data(), number_of_v_blocks*sizeof(ulint));

		V_select = {v_sel_size};
		in.read((char *)V_select.data(), v_sel_size*sizeof(ulint));

		R = {number_of_v_blocks};
		in.read((char *)R.data(), number_of_v_blocks*sizeof(ulint));

		for(auto bsd : BSDs)
			bsd.load(in);

		SEL = {number_of_t_blocks};
		in.read((char *)SEL.data(), number_of_t_blocks*sizeof(ulint));

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

		number_of_v_blocks = u/v + (u%v!=0);
		number_of_t_blocks = n/t + (n%t!=0);

		assert(number_of_v_blocks>0 and number_of_v_blocks<=u);

		//V rank in i-1
		ulint last_V_rank = 0;
		ulint global_rank = 0;

		{
			SEL = {number_of_t_blocks};
			//current '1'
			ulint j = 0;

			for(ulint i = 0;i<u;++i){

				if(B[i]){

					if(j%t==0){

						assert(j/t<number_of_t_blocks);
						SEL[j/t] = i/v;

					}

					j++;

				}

			}
		}

		for(ulint i = 0;i<number_of_v_blocks;++i){

			ulint l = i*v;
			ulint r = (i+1)*v;

			assert(l<u);

			if(r>u) r = u;

			vector<bool> subB = {B.begin()+l,B.begin()+r};

			R.push_back(global_rank);
			global_rank += number_of_1(subB);

			V_rank.push_back(last_V_rank);

			if(not empty(subB)){

				V_select.push_back(i);
				last_V_rank++;

				BSDs.push_back({subB,&D});

			}

		}

	}

	ulint number_of_1(vector<bool> &B){

		ulint res=0;

		for(auto b : B)
			res += (b?1:0);

		return res;

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

	ulint number_of_v_blocks=0;
	ulint number_of_t_blocks=0;

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
