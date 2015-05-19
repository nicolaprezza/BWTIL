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

#include "bsd_cgap.h"
#include "cgap_dictionary.h"
#include "packed_view.h"
#include "../common/common.h"

namespace bwtil{

class fid_cgap{

public:

	fid_cgap(){}

	fid_cgap(vector<bool> &B){

		if(B.size()==0) return; //nothing to do

		build(B);

	}

	ulint size(){
		return u;
	}

	ulint number_of_1(){
		return n;
	}

	/*
	 * \param i<number_of_1()
	 * \return position of the i-th bit set. i starts from 0 (first bit set has i=0)
	 */
	ulint select(ulint i){

		assert(t>0);
		assert(n>0);
		assert(u>0);

		assert(i<n);

		ulint q_l = SEL[i/t];
		ulint q_r = R_1.size();

		if(i/t+1<SEL.size())
			q_r = SEL[i/t+1]+1;

		//i-th bit set is among V-marked blocks q_l,...,q_r
		//note that |{q_l,...,q_r}| <= t, so following binary search costs log t

		assert(q_l<R_1.size());

		ulint q_m = (std::upper_bound(R_1.begin()+q_l,R_1.begin()+q_r,i) - R_1.begin()) - 1;

		assert(R_1[q_m]<=i);

		assert(q_m+1 == R_1.size() || R_1[q_m+1]>i);

		assert(q_m<V_select.size());

		//now V_select[q_m] is the block containing the i-th set bit

		assert(q_m<BSDs.size());
		assert(q_m<R_1.size());
		assert(i>=R_1[q_m]);

		assert(i-R_1[q_m]<BSDs[q_m].number_of_1());

		return V_select[q_m]*v + BSDs[q_m].select(i-R_1[q_m]);

	}

	/*
	 * \param i<=size() position in the bitvector
	 * \return number of '1' before position i excluded
	 */
	ulint rank(ulint i){

		assert(i<=u);

		if(u==0) return 0;

		assert(i/v<R.size());

		if(not V[i/v]) return R[i/v];

		assert(V_rank[i/v]<BSDs.size());

		assert(i%v <= BSDs[V_rank[i/v]].size());

		return R[i/v] + BSDs[V_rank[i/v]].rank(i%v);

	}

	/*
	 * returns i-th bit of the underlying bitvector
	 */
	bool operator[](ulint i){

		assert(i<u);

		if(not V[i/v]) return false;

		assert(V_rank[i/v]<BSDs.size());
		assert(i%v < BSDs[V_rank[i/v]].size());

		return BSDs[V_rank[i/v]][i%v];

	}

	/*
	 * retrieve length of the i-th gap (i>=0). gap length includes the leading 1
	 * \param i<number_of_1()
	 *
	 */
	ulint gapAt(ulint i){

		assert(t>0);
		assert(n>0);
		assert(u>0);

		assert(i<n);

		//we treat this case separately
		if(i==0) return select(0)+1;

		ulint q_l = SEL[i/t];
		ulint q_r = R_1.size();

		if(i/t+1<SEL.size())
			q_r = SEL[i/t+1]+1;

		//i-th bit set is among V-marked blocks q_l,...,q_r
		//note that |{q_l,...,q_r}| <= t, so following binary search costs log t

		assert(q_l<R_1.size());

		ulint q_m = (std::upper_bound(R_1.begin()+q_l,R_1.begin()+q_r,i) - R_1.begin()) - 1;

		assert(R_1[q_m]<=i);
		assert(q_m+1 == R_1.size() || R_1[q_m+1]>i);

		assert(q_m<V_select.size());

		//now V_select[q_m] is the block containing the i-th set bit

		assert(q_m<BSDs.size());
		assert(q_m<R_1.size());
		assert(i>=R_1[q_m]);

		assert(i-R_1[q_m]<BSDs[q_m].number_of_1());

		//if this is at least the 2nd gap in the BSD, then gap is not splitted between >=2 BSDs
		if(i>R_1[q_m])
			return BSDs[q_m].gapAt(i-R_1[q_m]);

		//otherwise, gap could be splitted between >=2 BSDs!
		//note that i>0 since we treated this case at the beginning
		return select(i)-select(i-1);

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

		ulint bsd_size = 0;

		for(auto bsd : BSDs)
			bsd_size += bsd.bytesize();

		return 	varsize +
				D.bytesize() +
				V_rank.size()*sizeof(ulint) +
				V_select.size()*sizeof(ulint) +
				R.size()*sizeof(ulint) +
				SEL.size()*sizeof(ulint) +
				bsd_size +
				V.size()/8 + sizeof(ulint) +
				R_1.size()*sizeof(ulint);

	}

	/*
	 * return the bytesize of the Huffman-compressed sequence
	 */
	ulint C_bytesize(){

		ulint res=0;
		for(auto bsd : BSDs)
			res += bsd.C_bytesize();

		return res;

	}

	/*
	 * return the bytesize of the dictionary
	 */
	ulint D_bytesize(){

		return D.bytesize();

	}

	ulint first_el_bytesize(){

		ulint res=0;
		for(auto bsd : BSDs)
			res += bsd.first_el_bytesize();

		return res;

	}

	ulint C_addr_bytesize(){

		ulint res=0;
		for(auto bsd : BSDs)
			res += bsd.C_addr_bytesize();

		return res;

	}

	ulint fid_arrays_bytesize(){
		return 	V_rank.size()*sizeof(ulint) +
				V_select.size()*sizeof(ulint) +
				R.size()*sizeof(ulint) +
				SEL.size()*sizeof(ulint) +
				V.size()/8 + sizeof(ulint) +
				R_1.size()*sizeof(ulint);
	}

	uint get_prefix_length(){
		return D.get_prefix_length();
	}

	/* serialize the structure to the ostream
	 * \param out	 the ostream
	 */
	ulint serialize(std::ostream& out){

		out.write((char *)&W, sizeof(ulint));

		if(W==0) return sizeof(ulint);

		out.write((char *)&u, sizeof(ulint));
		out.write((char *)&n, sizeof(ulint));
		out.write((char *)&v, sizeof(ulint));
		out.write((char *)&t, sizeof(ulint));
		out.write((char *)&number_of_v_blocks, sizeof(ulint));
		out.write((char *)&number_of_t_blocks, sizeof(ulint));

		ulint v_sel_size = V_select.size();
		out.write((char *)&v_sel_size, sizeof(ulint));

		ulint bsds_size = BSDs.size();
		out.write((char *)&bsds_size, sizeof(ulint));

		ulint w_bytes = 8*sizeof(ulint);

		D.serialize(out);

		for(ulint i=0;i<number_of_v_blocks;++i){
			bool b = V[i];
			out.write((char *)&b, sizeof(bool));
		}

		w_bytes += number_of_v_blocks*sizeof(bool);

		out.write((char *)V_rank.data(), number_of_v_blocks*sizeof(ulint));
		w_bytes += number_of_v_blocks*sizeof(ulint);

		out.write((char *)V_select.data(), v_sel_size*sizeof(ulint));
		w_bytes += v_sel_size*sizeof(ulint);

		out.write((char *)R.data(), number_of_v_blocks*sizeof(ulint));
		w_bytes += number_of_v_blocks*sizeof(ulint);

		out.write((char *)R_1.data(), v_sel_size*sizeof(ulint));
		w_bytes += v_sel_size*sizeof(ulint);

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

		if(W==0) return;

		in.read((char *)&u, sizeof(ulint));
		in.read((char *)&n, sizeof(ulint));
		in.read((char *)&v, sizeof(ulint));
		in.read((char *)&t, sizeof(ulint));
		in.read((char *)&number_of_v_blocks, sizeof(ulint));
		in.read((char *)&number_of_t_blocks, sizeof(ulint));

		ulint v_sel_size;
		in.read((char *)&v_sel_size, sizeof(ulint));

		ulint bsds_size;
		in.read((char *)&bsds_size, sizeof(ulint));

		D.load(in);

		V = vector<bool>(number_of_v_blocks);
		for(ulint i=0;i<number_of_v_blocks;++i){
			bool b;
			in.read((char *)&b, sizeof(bool));
			V[i]=b;
		}

		V_rank = vector<ulint>(number_of_v_blocks);
		in.read((char *)V_rank.data(), number_of_v_blocks*sizeof(ulint));

		V_select = vector<ulint>(v_sel_size);
		in.read((char *)V_select.data(), v_sel_size*sizeof(ulint));

		R = vector<ulint>(number_of_v_blocks);
		in.read((char *)R.data(), number_of_v_blocks*sizeof(ulint));

		R_1 = vector<ulint>(v_sel_size);
		in.read((char *)R_1.data(), v_sel_size*sizeof(ulint));

		BSDs = vector<bsd_cgap>(bsds_size,bsd_cgap(&D));
		for(ulint i=0;i<BSDs.size();++i)
			BSDs[i].load(in);

		SEL = vector<ulint>(number_of_t_blocks);
		in.read((char *)SEL.data(), number_of_t_blocks*sizeof(ulint));

	}

	fid_cgap & operator= (const fid_cgap & other) {

		W=other.W;

		//number of bits/number of 1s
		u=other.u;
		n=other.n;

		//block size
		v=other.v;

		//sample select every t bits set
		t=other.t;

		number_of_v_blocks=other.number_of_v_blocks;
		number_of_t_blocks=other.number_of_t_blocks;

		//Dictionary: supports encoding/decoding of gap codes (Huffman)
		D=other.D;

		//naive (but ultrafast) constant-time rank-select bitvector taking 2ulog u bits
		V = other.V;
		V_rank = other.V_rank;
		V_select = other.V_select;

		//sampled rank results
		R = other.R;

		//same as R, but we store R entries only in blocks marked with a 1 in V
		R_1 = other.R_1;

		//one BSD per nonempty block
		BSDs = other.BSDs;

		for(ulint i=0;i<BSDs.size();++i)
			BSDs[i].replace_dictionary(&D);

		//sampled select on blocks
		SEL = other.SEL;

	    return *this;
	}

	double entropy(){

		double H=0;
		map<ulint,ulint> freqs;

		for(ulint i=0;i<n;++i)
			freqs[gapAt(i)]++;

		for(auto f : freqs)
			H += f.second * log2((double)n/(double)f.second);

		return H/n;

	}

	double number_of_distinct_gaps(){

		set<ulint> gaps;

		for(ulint i=0;i<n;++i)
			gaps.insert(gapAt(i));

		return gaps.size();

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
		v = (u*intlog2(u)*64*5)/(n==0?1:n);
		if(v>u) v=u;

		t = intlog2(u)*intlog2(u)*5;

		number_of_v_blocks = u/v + (u%v!=0);
		number_of_t_blocks = n/t + (n%t!=0);

		assert(number_of_v_blocks>0 and number_of_v_blocks<=u);
		//assert(number_of_t_blocks>0);

		//V rank in i-1
		ulint last_V_rank = 0;
		ulint global_rank = 0;

		for(ulint i = 0;i<number_of_v_blocks;++i){

			ulint l = i*v;
			ulint r = (i+1)*v;

			assert(l<u);

			if(r>u) r = u;

			auto subB = vector<bool>(r-l,false);
			for(ulint j=l;j<r;++j)
				subB[j-l] = B[j];

			R.push_back(global_rank);
			V_rank.push_back(last_V_rank);

			if(not empty(subB)){

				R_1.push_back(global_rank);
				V.push_back(true);
				V_select.push_back(i);
				last_V_rank++;

				BSDs.push_back(bsd_cgap(subB,&D));

			}else{

				V.push_back(false);

			}

			global_rank += number_of_1(subB);

		}

		{
			SEL = vector<ulint>(number_of_t_blocks);

			//current '1'
			ulint j = 0;

			for(ulint i = 0;i<u;++i){

				if(B[i]){

					if(j%t==0){

						assert(j/t<SEL.size());
						assert(i/v<V_rank.size());
						SEL[j/t] = V_rank[i/v];

					}

					j++;

				}

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
	vector<bool> V;
	vector<ulint> V_rank;
	vector<ulint> V_select;

	//sampled rank results
	vector<ulint> R;

	//same as R, but we store R entries only in blocks marked with a 1 in V
	vector<ulint> R_1;

	//one BSD per nonempty block
	vector<bsd_cgap> BSDs;

	//sampled select on blocks
	vector<ulint> SEL;

};

}

#endif /* FID_CGAP_H_ */
