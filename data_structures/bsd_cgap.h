/*
 * bsd_cgap.h
 *
 *  Created on: May 5, 2015
 *      Author: nicola
 *
 *  Binary searchable dictionary using entropy-compressed gaps.
 *  This is the building block of the fully indexable dictionary fid_cgap
 */

#ifndef BSD_CGAP_H_
#define BSD_CGAP_H_

#include "cgap_dictionary.h"
#include "bitview.h"
#include "packed_view.h"
#include "../common/common.h"

namespace bwtil{

class bsd_cgap{

public:

	bsd_cgap(){}

	/*
	 * Constructor 1 : build BSD from a bitvector
	 * Builds its own dictionary.
	 */
	bsd_cgap(vector<bool> B){

		bool last = B[B.size()-1];
		auto gaps = cgap_dictionary::bitvector_to_gaps(B);
		auto D = cgap_dictionary::build_dictionary(gaps);

		build(gaps,last,D);

	}

	/*
	 * Constructor 2: build BSD from a bitvector and a dictionary.
	 * copy reference to passed dictionary (do not create a new one)
	 */
	bsd_cgap(vector<bool> B,cgap_dictionary &D){

		bool last = B[B.size()-1];
		auto gaps = cgap_dictionary::bitvector_to_gaps(B);

		build(gaps,last,D);

	}

	/*
	 * Constructor 3:
	 * build BSD data structure given the ordered list of gaps, value of last bit, and the gap dictionary
	 * Note: let k be the last gap length. The tail of the bitvector is 0^k or 0^(k-1)1 if last=0 or
	 * last=1, respectively.
	 *
	 */
	bsd_cgap(vector<ulint> &gaps,bool last, cgap_dictionary &D){
		build(gaps,last,D);
	}

	void build(vector<ulint> &gaps,bool last, cgap_dictionary &D){

		W = sizeof(ulint)*8;

		this->D=D;
		if(last) last=1;

		if(last){

			tail=0;
			n=gaps.size();

		}else{

			tail=gaps[gaps.size()-1];
			n=gaps.size()-1;
		}

		u = tail;

		//count bits and ones
		for(ulint i=0;i<gaps.size()-1;++i)
			u+=gaps[i];

		logu = intlog2(u);

		//compute block size
		t = logu;
		if(t<2) t = 2;

		number_of_blocks = n/t + (n%t!=0);
		first_el = {logu,number_of_blocks};

		c=0;

		//count number of bits in C
		for(ulint i=0;i<(gaps.size()-1)+last;++i){

			if(i%t!=0)
				c+=D.encode(gaps[i]).size();

		}

		C = {c};
		logc = intlog2(c);

		C_addr = {logc,number_of_blocks};

		ulint cumulative_u = 0;
		ulint cumulative_c = 0;

		for(ulint i=0;i<(gaps.size()-1)+last;++i){

			cumulative_u+=gaps[i];

			if(i%t==0){

				first_el[i/t] = cumulative_u-1;
				C_addr[i/t] = cumulative_c;

			}else{

				auto code = D.encode(gaps[i]);
				uint l = code.size();

				ulint code_ul=0;

				for(uint j=0;j<l;j++)
					code_ul = code_ul*2+code[j];

				//transform to access correctly C:
				//interval [i,i+l) -> [(c-i)-l,c-i)
				ulint left = (c - cumulative_c)-l;
				ulint right = c - cumulative_c;

				C(left,right) = code_ul;

				cumulative_c+=l;

			}

		}

	}

	ulint size(){
		return u+tail;
	}

	ulint number_of_1(){
		return n;
	}

	/*
	 * \param i<=size() position in the bitvector
	 * \return number of '1' before position i excluded
	 */
	ulint rank(ulint i){

		assert(i<=size());

		//i falls in the tail of zeroes
		if(i>=u) return number_of_1();

		//from here, i < u

		return 0;

	}

	/*
	 * \param i<number_of_1()
	 * \return position of the i-th bit set. i starts from 0 (first bit set has i=0)
	 */
	ulint select(ulint i){

		assert(i<number_of_1());

		ulint bl = i/t;
		ulint rem = i%t;

		ulint result = first_el[bl];
		ulint C_idx = C_addr[bl];

		//now extract i%t gaps starting from position C_idx
		for(ulint i=0;i<rem;++i){

			//extract gap code, decompress it and retrieve its bit-length
			auto g = extract_gap(C_idx);

			result += g.first;//decompressed gap value
			C_idx += g.second;//bit length of the code

		}

		return result;

	}

private:

	/*
	 * extract gap starting from the bit C[i]
	 * \param i position in C
	 * \return <gap,len> : gap value and length in bits of the code
	 */
	pair <ulint,ulint> extract_gap(ulint i){

		//transform to access correctly C:
		//interval [i,i+l) -> [(c-i)-l,c-i)

		//how many bits have to be extracted? W bits or less if we go beyond C end
		ulint len = (i + W>c?c:i + W) - i;

		//start and end position in C
		ulint left = (c - i)-len;
		ulint right = c - i;

		//get bits
		ulint x = C.get(left,right);
		//left shift: align extracted code on the left
		x = x << (W-len);

		//decode and return
		return D.decode(x);

	}

	//Dictionary: permits encoding/decoding of gap codes (Huffman)
	cgap_dictionary D;

	//bit-array with all Huffman codes stored consecutively
	bitview<vector> C;

	//word size
	uint8_t W=0;

	//bit-size of C
	ulint c=0;

	//size of a pointer in C
	uint8_t logc;
	//number of bits required to write u
	uint8_t logu;

	//length of the tail of zeroes in the end of the bitvector.
	//u does not account for this tail!
	//if tail=0, then last bit is 1.
	ulint tail=0;

	//number of bits/number of 1s
	ulint u=0,n=0;

	//block size. Each block contains t 1s. Equals log u
	uint t=0;

	ulint number_of_blocks=0;

	//for each block, explicit 1st element
	packed_view<vector> first_el;

	//for each block, starting position in C of the 2nd element
	packed_view<vector> C_addr;

};

}

#endif /* BSD_CGAP_H_ */
