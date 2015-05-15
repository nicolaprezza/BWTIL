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

	bsd_cgap(cgap_dictionary * D){
		this->D = D;
	}

	/*
	 * Constructor 1: build BSD from a bitvector and a dictionary.
	 * copy reference to passed dictionary (do not create a new one)
	 */
	bsd_cgap(vector<bool> &B,cgap_dictionary * D){

		bool last = B[B.size()-1];
		auto gaps = cgap_dictionary::bitvector_to_gaps(B);

		build(gaps,last,D);

	}

	/*
	 * Constructor 2:
	 * build BSD data structure given the ordered list of gaps, value of last bit, and the gap dictionary
	 * Note: let k be the last gap length. The tail of the bitvector is 0^k or 0^(k-1)1 if last=0 or
	 * last=1, respectively.
	 *
	 */
	bsd_cgap(vector<ulint> &gaps,bool last, cgap_dictionary * D){
		build(gaps,last,D);
	}

	ulint size(){
		assert(D!=0);
		return u;
	}

	ulint number_of_1(){
		assert(D!=0);
		return n;
	}

	/*
	 * \param i<number_of_1()
	 * \return position of the i-th bit set. i starts from 0 (first bit set has i=0)
	 */
	ulint select(ulint i){

		assert(D!=0);
		assert(u!=0);

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

	/*
	 * \param i<=size() position in the bitvector
	 * \return number of '1' before position i excluded
	 */
	ulint rank(ulint i){

		assert(D!=0);
		assert(u!=0);

		assert(i<=size());

		//i falls in the tail of zeroes
		if(i>=(u-tail)) return number_of_1();

		//from here, i < (u-tail)

		//i falls before (or in the same position of) the first 1
		if(i<=first_el[0])
			return 0;

		//binary search explicitly stored first block elements

		//first block whose first element is > i
		auto it = std::upper_bound(first_el.begin(),first_el.end(),i);

		//block number
		ulint bl = std::distance(first_el.begin(),it) - 1;

		ulint result = t*bl;
		ulint C_idx = C_addr[bl];
		ulint u_pos = first_el[bl];

/*		cout << "start from block " << bl << endl;
		cout << "initial rank is " << result << endl;
		cout << "initial u pos is " << u_pos << endl;*/

		//extract gaps until we reach i
		ulint last_gap = 0;
		uint steps = 0;

		while(u_pos+last_gap<i and steps<t-1){

			steps++;
			result++;
			u_pos += last_gap;

			//extract gap code, decompress it and retrieve its bit-length
			auto g = extract_gap(C_idx);

			last_gap = g.first;//decompressed gap value
			C_idx += g.second;//bit length of the code

			//cout << "extracted gap " << last_gap << endl;

		}

		//in this case, i falls in the next block before the first 1
		if(steps==t-1 and u_pos+last_gap<i)
			result = (bl+1)*t;

		return result;

	}

	/*
	 * returns i-th bit of the underlying bitvector
	 */
	bool operator[](ulint i){

		assert(D!=0);
		assert(u!=0);
		assert(i<size());

		//i falls in the tail of zeroes
		if(i>=(u-tail)) return false;

		//from here, i < (u-tail)

		//i falls before or on the first 1
		if(i<=first_el[0])
			return i==first_el[0];

		//binary search explicitly stored first block elements

		//first block whose first element is > i
		auto it = std::upper_bound(first_el.begin(),first_el.end(),i);

		//block number
		ulint bl = std::distance(first_el.begin(),it) - 1;

		ulint C_idx = C_addr[bl];
		ulint u_pos = first_el[bl];

		//cout << "start from block " << bl << endl;
		//cout << "initial u pos is " << u_pos << endl;

		//extract gaps until we reach i
		ulint last_gap = 0;
		uint steps = 0;

		while((u_pos+last_gap)<=i and steps<t-1){

			if((u_pos+last_gap)==i)
				return true;

			steps++;
			u_pos += last_gap;

			//extract gap code, decompress it and retrieve its bit-length
			auto g = extract_gap(C_idx);

			last_gap = g.first;//decompressed gap value
			C_idx += g.second;//bit length of the code

		}

		return (u_pos+last_gap)==i;

	}

	/*
	 * retrieve length of the i-th gap (i>=0). gap length includes the leading 1
	 * \param i<number_of_1()
	 *
	 */
	ulint gapAt(ulint i){

		assert(D!=0);
		assert(u!=0);
		assert(i<number_of_1());

		if(i==0)
			return first_el[0]+1;

		ulint bl = i/t;
		ulint rem = i%t;

		if(rem==0)
			return first_el[bl] - select(i-1);

		ulint result = first_el[bl];
		ulint C_idx = C_addr[bl];

		//now extract i%t gaps starting from position C_idx
		for(ulint i=0;i<rem;++i){

			//extract gap code, decompress it and retrieve its bit-length
			auto g = extract_gap(C_idx);

			result = g.first;//decompressed gap value
			C_idx += g.second;//bit length of the code

		}

		return result;

	}

	/*
	 * \return bit size of the data structure
	 */
	ulint bytesize(){

		assert(D!=0);
		assert(u!=0);

		ulint C_size = C.container().size()*sizeof(ulint);
		ulint first_el_size = first_el.container().size()*sizeof(ulint);
		ulint C_addr_size = C_addr.container().size()*sizeof(ulint);

		//we do not count dictionary size because is built externally
		//ulint D_size = D->bytesize();

		ulint varsize = (	sizeof(W) +
							sizeof(c) +
							sizeof(logc) +
							sizeof(logu) +
							sizeof(tail) +
							sizeof(u) +
							sizeof(n) +
							sizeof(t) +
							sizeof(number_of_blocks));

		return 	C_size +
				first_el_size +
				C_addr_size +
				varsize;

	}

	ulint first_el_bytesize(){
		return first_el.container().size()*sizeof(ulint);
	}

	ulint C_addr_bytesize(){
		return C_addr.container().size()*sizeof(ulint);
	}

	/*
	 * return the bytesize of the Huffman-compressed sequence
	 */
	ulint C_bytesize(){
		return C.container().size()*sizeof(ulint);
	}


	/* serialize the structure to the ostream
	 * \param out	 the ostream
	 */
	ulint serialize(std::ostream& out){

		assert(D!=0);
		assert(u!=0);

		//Dictionary is not serialized, since it's built externally

		ulint w_bytes = 0;

		//vars

		out.write((char *)&W, sizeof(uint8_t));
		out.write((char *)&c, sizeof(ulint));
		out.write((char *)&logc, sizeof(uint8_t));
		out.write((char *)&logu, sizeof(uint8_t));
		out.write((char *)&tail, sizeof(ulint));
		out.write((char *)&u, sizeof(ulint));
		out.write((char *)&n, sizeof(ulint));
		out.write((char *)&t, sizeof(uint));
		out.write((char *)&number_of_blocks, sizeof(ulint));

		w_bytes += sizeof(uint8_t)*4 + sizeof(ulint)*5 + sizeof(uint);

		ulint bitview_type_size = sizeof(*C.container().data());
		out.write((char *)&bitview_type_size, sizeof(ulint));
		w_bytes += sizeof(ulint);

		ulint packed_view_type_size = sizeof(*first_el.container().data());
		out.write((char *)&packed_view_type_size, sizeof(ulint));
		w_bytes += sizeof(ulint);

		// C

		ulint C_container_len = C.container().size();
		out.write((char *)&C_container_len, sizeof(ulint));
		w_bytes += sizeof(ulint);

		out.write((char *)C.container().data(), bitview_type_size*C_container_len);
		w_bytes += bitview_type_size*C_container_len;

		//first_el

		ulint first_el_container_len = first_el.container().size();
		out.write((char *)&first_el_container_len, sizeof(ulint));
		w_bytes += sizeof(ulint);

		out.write((char *)first_el.container().data(), packed_view_type_size*first_el_container_len);
		w_bytes += packed_view_type_size*first_el_container_len;

		//C_addr

		ulint C_addr_container_len = C_addr.container().size();
		out.write((char *)&C_addr_container_len, sizeof(ulint));
		w_bytes += sizeof(ulint);

		out.write((char *)C_addr.container().data(), packed_view_type_size*C_addr_container_len);
		w_bytes += packed_view_type_size*C_addr_container_len;

		return w_bytes;

	}

	/* load the structure from the istream
	 * \param in the istream
	 */
	void load(std::istream& in) {

		//must load an object that already has a dictionary!
		assert(D!=0);

		//Dictionary is not serialized, since it's built externally

		//vars

		in.read((char *)&W, sizeof(uint8_t));
		in.read((char *)&c, sizeof(ulint));
		in.read((char *)&logc, sizeof(uint8_t));
		in.read((char *)&logu, sizeof(uint8_t));
		in.read((char *)&tail, sizeof(ulint));
		in.read((char *)&u, sizeof(ulint));
		in.read((char *)&n, sizeof(ulint));
		in.read((char *)&t, sizeof(uint));
		in.read((char *)&number_of_blocks, sizeof(ulint));

		ulint bitview_type_size;
		in.read((char *)&bitview_type_size, sizeof(ulint));

		ulint packed_view_type_size;
		in.read((char *)&packed_view_type_size, sizeof(ulint));

		// C

		ulint C_container_len;
		in.read((char *)&C_container_len, sizeof(ulint));

		C = bitview<vector>(c);
		in.read((char *)C.container().data(), bitview_type_size*C_container_len);

		//first_el

		ulint first_el_container_len;
		in.read((char *)&first_el_container_len, sizeof(ulint));

		first_el = packed_view<vector>(logu,number_of_blocks);
		in.read((char *)first_el.container().data(), packed_view_type_size*first_el_container_len);

		//C_addr

		ulint C_addr_container_len;
		in.read((char *)&C_addr_container_len, sizeof(ulint));

		C_addr = packed_view<vector>(logc,number_of_blocks);
		in.read((char *)C_addr.container().data(), packed_view_type_size*C_addr_container_len);

	}

	void replace_dictionary(cgap_dictionary * D){
		this->D = D;
	}

private:

	void build(vector<ulint> &gaps,bool last, cgap_dictionary * D){

		assert(gaps.size()>0);

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

		//cout << "n = " << (ulint)n<<endl;

		u = tail;

		//count bits and ones
		for(ulint i=0;i<(gaps.size()-1)+last;++i)
			u+=gaps[i];

		assert(u>0);

		//cout << "u = " << (ulint)u<<endl;

		logu = intlog2(u);

		//compute block size
		t = 5*logu;
		if(t<2) t = 2;

		number_of_blocks = n/t + (n%t!=0);
		first_el = packed_view<vector>(logu,number_of_blocks);

		c=0;

		//count number of bits in C
		for(ulint i=0;i<(gaps.size()-1)+last;++i){

			if(i%t!=0)
				c+=D->encode(gaps[i]).size();

		}

		C = bitview<vector>(c);
		logc = intlog2(c);

		//cout << "log c = " << (ulint)logc<<endl;
		//cout << "log u = " << (ulint)logu<<endl;
		//cout << "t = " << (ulint)t<<endl;

		C_addr = packed_view<vector>(logc,number_of_blocks);

		ulint cumulative_u = 0;
		ulint cumulative_c = 0;

		for(ulint i=0;i<(gaps.size()-1)+last;++i){

			cumulative_u+=gaps[i];

			if(i%t==0){

				assert(cumulative_u>0);
				assert(cumulative_u-1<(ulint(1)<<logu));
				assert(cumulative_c<(ulint(1)<<logc));

				first_el[i/t] = cumulative_u-1;
				C_addr[i/t] = cumulative_c;

			}else{

				auto code = D->encode(gaps[i]);
				uint l = code.size();

				ulint code_ul=0;

				for(uint j=0;j<l;j++)
					code_ul = code_ul*2+code[j];

				assert(code_ul<(ulint(1)<<l));

				//transform to access correctly C:
				//interval [i,i+l) -> [(c-i)-l,c-i)

				assert(c>=(cumulative_c+l));
				ulint left = (c - cumulative_c)-l;
				ulint right = c - cumulative_c;

				C(left,right) = code_ul;

				cumulative_c+=l;

			}

		}

		//cout << "c = " << (ulint)c<<endl;

		/*cout << "first el : "<<endl;
		for(auto i : first_el)
			cout << i << endl;*/

	}

	/*
	 * extract gap starting from the bit C[i]
	 * \param i position in C
	 * \return <gap,len> : gap value and length in bits of the code
	 */
	pair <ulint,ulint> extract_gap(ulint i){

		assert(D!=0);
		assert(u!=0);

		//transform to access correctly C:
		//interval [i,i+l) -> [(c-i)-l,c-i)

		//how many bits have to be extracted? W-1 bits or less if we go beyond C end
		ulint len = (i + W - 1 >c?c:i + W - 1) - i;

		//start and end position in C

		assert(c>=(i+len));

		ulint left = (c - i)-len;
		ulint right = c - i;

		//get bits
		assert(len<W);

		ulint x = C.get(left,right);
		//left shift: align extracted code on the left
		x = x << (W-len);

		//decode and return
		return D->decode(x);

	}

	//Dictionary: supports encoding/decoding of gap codes (Huffman)
	cgap_dictionary * D = 0;

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
