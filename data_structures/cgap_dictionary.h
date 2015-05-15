/*
 * cgap_dictionary.h
 *
 *  Created on: May 5, 2015
 *      Author: nicola
 *
 *  Huffman dictionary for the cGAP data structure
 *
 */

#ifndef CGAP_DICTIONARY_H_
#define CGAP_DICTIONARY_H_

#include "HuffmanTree.h"
#include "succinct_bitvector.h"
#include "packed_view.h"
#include "../common/common.h"
#include <set>
#include <map>

namespace bwtil{

class cgap_dictionary{

	typedef std::tuple<ulint,ulint,uint8_t> triple;

public:

	cgap_dictionary(){}

	/*
	 * \param gaps: distinct gap lengths and their absolute frequencies (pairs: <gap length,freq>)
	 *
	 * note: gap length includes the leading 1. es: in 00100011 gap lengths are 3,4,1
	 */
	cgap_dictionary(vector<pair<ulint,ulint> > gaps){

		assert(gaps.size()>0);

		//if there is only 1 distinct gap, this creates problems with the huffman codes ...
		//in this case, create one fake gap. There will be 2 codes: 0 and 1
		if(gaps.size()==1)
			gaps.push_back({gaps[0].first+1,1});

		//compute u and n
		ulint u=0;
		ulint n=0;

		ulint max_gap=0;

		for(auto g : gaps){

			assert(g.first>0);
			assert(g.second>0);

			u += (g.first*g.second);
			n += g.second;

			if(g.first>max_gap)
				max_gap=g.first;

		}

		log2_max_gap = intlog2(max_gap);

		//build Huffman tree and retrieve boolean codes
		vector<vector<bool> > codes;

		{
			vector<ulint> freqs;

			for(auto g : gaps)
				freqs.push_back(g.second);

			HuffmanTree<ulint> ht(freqs);

			codes = ht.getCodes();

			for(auto c : codes){

				//make sure that no code is longer than 63 bits
				assert(c.size()<sizeof(ulint)*8);

			}

		}

		assert(codes.size()==gaps.size());

		{
			//count long codes
			vector<ulint> long_codes(64,0);
			for(auto c : codes)
				for(ulint i=0;i<c.size();++i)
					long_codes[i]++;

			//just for testing purposes: code_freq[i] contains the cumulative
			//frequency of codes having length <= i
			vector<double> code_freq(64,0);
			for(ulint i=0;i<codes.size();++i)
				code_freq[codes[i].size()]+=gaps[i].second;
			for(ulint i=1;i<code_freq.size();++i)
				code_freq[i] += code_freq[i-1];
			for(ulint i=0;i<code_freq.size();++i)
				code_freq[i] = 100*code_freq[i]/n;

			//compute optimal prefix length for the hash table,
			//i.e. prefix length that minimizes overall size of the dictionary

			prefix_length=1;
			ulint opt_bitsize = ulint(1)<<63;

			//for efficiency reasons, we limit the maximum prefix length to 30 (in order to avoid
			//huge dictionaries)
			for(ulint p=1;p<30;++p){

				//cout << p << " " << code_freq[p] << " " << long_codes[p] << " " << assess_bitsize(p,long_codes[p]) << endl;

				if(assess_bitsize(p,long_codes[p])<opt_bitsize){

					opt_bitsize = assess_bitsize(p,long_codes[p]);
					prefix_length = p;

				}

			}

			//now choose largest prefix that exceeds at most 20% the optimal bitsize
			ulint max_allowed_bitsize = opt_bitsize + (20*opt_bitsize)/100;

			//cout << "opt p = " << (uint)prefix_length << endl;
			//cout << "opt bitsize = " << opt_bitsize << endl;
			//cout << "max bitsize = " << max_allowed_bitsize << endl;

			for(ulint p=1;p<30;++p){

				if(assess_bitsize(p,long_codes[p])<=max_allowed_bitsize)
					prefix_length = p;

			}

			//cout << "optimal prefix_length = " << (ulint)prefix_length<<endl;
			//cout << "with the hash table we catch " << code_freq[prefix_length] << "% of the codes!"<<endl;

		}

		//build the code function
		for(ulint i=0;i<codes.size();++i)
			encoding[gaps[i].first] = codes[i];

		ulint H_size = ulint(1)<<prefix_length;

		H_val = packed_view<vector>(log2_max_gap,H_size);
		H_len = packed_view<vector>(intlog2(prefix_length),H_size);

		for(ulint i=0;i<H_size;++i) H_len[i] = 0;

		//build hash

		//store here triples of the form <code,gap value,code length>
		//where code is the left-shifted Huffman code

		for(ulint i=0;i<gaps.size();++i){

			//code length
			auto l = codes[i].size();
			assert(l>0);

			if(l<=prefix_length){

				//the hash is sufficient to decode this code. Compute H entry
				ulint h = 0;

				for(auto b : codes[i])
					h = h*2+b;

				//shift to left so to occupy exactly prefix_length bits
				h = h << (prefix_length-l);

				//fill all combinations of h followed by any bit sequence (in total prefix_length bits)
				for(ulint j=0;j<(ulint(1)<<(prefix_length-l));++j){
					H_val[h+j] = gaps[i].first;
					H_len[h+j] = l;
				}

			}else{

				//pack the code in a memory word
				ulint h = 0;

				for(uint j=0;j<codes[i].size();++j)
					h = h*2 + codes[i][j];

				//left-shift
				h = h << (sizeof(ulint)*8-codes[i].size());

				H_long.push_back(triple(h,gaps[i].first,(uint8_t)codes[i].size()));

			}

		}

		auto comp = [](triple x, triple y){ return std::get<0>(x) < std::get<0>(y); };
		std::sort(H_long.begin(),H_long.end(),comp);

		assert(log2_max_gap>0);

	}

	uint get_prefix_length(){
		return prefix_length;
	}

	/*
	 * Decode the code in the leftmost part of x
	 * \param x unsigned int containing the code in the most significant bits
	 * \return pair <decoded value(i.e. gap length), bit length of the code>
	 */
	pair<ulint, ulint> decode(ulint x){

		//take leftmost prefix_length bits

		uint W = sizeof(ulint)*8;
		ulint h = x >> (W-prefix_length);

		if(H_len[h]==0){

			auto comp = [](triple a, triple b){ return std::get<0>(a) < std::get<0>(b); };

			auto t = triple(x,0,0);

			ulint i = std::upper_bound(H_long.begin(),H_long.end(),t, comp) - H_long.begin();

			assert(i>0);

			i--;

			return {std::get<1>(H_long[i]),std::get<2>(H_long[i])};

		}

		//does not exceed: just access H
		return {H_val[h],H_len[h]};

	}

	/*
	 * Decode the input code
	 * \param vb the code
	 * \return pair <decoded value(i.e. gap length), bit length of the code>
	 */
	pair<ulint, ulint> decode(vector<bool> vb){

		ulint x = 0;

		for(auto b : vb)
			x = x*2+b;

		uint W = sizeof(ulint)*8;
		x = x << (W-vb.size());

		return decode(x);

	}

	vector<bool> encode(ulint g){

		/*
		 * warning: for space efficiency, encoding is not serialized!!
		 * therefore, if you load the structure from disk, this assert will fail
		 */
		assert(encoding.size()>0);
		return encoding[g];

	}

	/*
	 * build (Huffman) dictionary given list of gaps.
	 */
	static cgap_dictionary build_dictionary(vector<ulint> gaps){

		assert(gaps.size()>0);

		auto comp = [](pair<ulint,ulint> x, pair<ulint,ulint> y){ return x.first < y.first; };
		std::set<pair<ulint,ulint> ,decltype(comp)> gaps_and_freq(comp);

		for(auto g : gaps){

			if(gaps_and_freq.find({g,0})!=gaps_and_freq.end()){

				auto it = gaps_and_freq.find({g,0});
				ulint new_freq = it->second+1;
				gaps_and_freq.erase(it);
				gaps_and_freq.insert({g,new_freq});

			}else{
				gaps_and_freq.insert({g,1});
			}

		}

		vector<pair<ulint,ulint> > gaps_and_freq_v;

		for(auto p : gaps_and_freq)
			gaps_and_freq_v.push_back(p);

		return {gaps_and_freq_v};

	}

	/*
	 * convert a bitvector to a list of gaps.
	 * Length of a gap is number of 0s + 1 (we count the leading bit set)
	 * If last bit is 0, last gap equals
	 * the length of the tail of 0s in B
	 */
	static vector<ulint> bitvector_to_gaps(vector<bool> &B){

		ulint gap_len=1;
		vector<ulint> gaps;

		for(ulint i=0;i<B.size();++i){

			if(B[i]){

				gaps.push_back(gap_len);
				gap_len=1;

			}else{
				gap_len++;
			}

		}

		if(not B[B.size()-1])
			gaps.push_back(gap_len-1);

		assert(gaps.size()>0);

		return gaps;

	}

	ulint bytesize(){

		ulint H_size = 	H_val.container().size()*sizeof(ulint) +
						H_len.container().size()*sizeof(ulint);

		ulint H_long_size = H_long.size()*(sizeof(triple));

		ulint varsize = sizeof(prefix_length) + sizeof(log2_max_gap);

		return 	H_size +
				H_long_size +
				varsize;

	}

	ulint serialize(std::ostream& out){

		out.write((char *)&log2_max_gap, sizeof(ulint));
		ulint w_bytes = sizeof(ulint);

		//if empty dictionary, stop here
		if(log2_max_gap==0) return w_bytes;

		out.write((char *)&prefix_length, sizeof(uint8_t));
		w_bytes += sizeof(uint8_t);

		ulint H_size = H_val.size();
		out.write((char *)&H_size, sizeof(ulint));
		w_bytes += sizeof(ulint);

		ulint H_long_size = H_long.size();
		out.write((char *)&H_long_size, sizeof(ulint));
		w_bytes += sizeof(ulint);

		ulint H_val_cont = H_val.container().size();
		out.write((char *)&H_val_cont, sizeof(ulint));
		w_bytes += sizeof(ulint);

		ulint H_len_cont = H_len.container().size();
		out.write((char *)&H_len_cont, sizeof(ulint));
		w_bytes += sizeof(ulint);

		out.write((char *)H_long.data(), H_long_size*sizeof(triple));
		w_bytes += H_long_size*sizeof(triple);

		out.write((char *)H_val.container().data(), H_val_cont*sizeof(ulint));
		w_bytes += H_val_cont*sizeof(ulint);

		out.write((char *)H_len.container().data(), H_len_cont*sizeof(ulint));
		w_bytes += H_len_cont*sizeof(ulint);

		return w_bytes;

	}

	void load(std::istream& in) {

		in.read((char *)&log2_max_gap, sizeof(ulint));

		//if empty dictionary, stop here
		if(log2_max_gap==0) return;

		in.read((char *)&prefix_length, sizeof(uint8_t));

		ulint H_size;
		in.read((char *)&H_size, sizeof(ulint));

		ulint H_long_size;
		in.read((char *)&H_long_size, sizeof(ulint));

		ulint H_val_cont;
		in.read((char *)&H_val_cont, sizeof(ulint));

		ulint H_len_cont;
		in.read((char *)&H_len_cont, sizeof(ulint));

		H_val = packed_view<vector>(log2_max_gap,H_size);
		H_len = packed_view<vector>(intlog2(prefix_length),H_size);
		H_long = vector<triple>(H_long_size);

		in.read((char *)H_long.data(), H_long_size*sizeof(triple));
		in.read((char *)H_val.container().data(), H_val_cont*sizeof(ulint));
		in.read((char *)H_len.container().data(), H_len_cont*sizeof(ulint));

	}

private:

	ulint assess_bitsize(ulint p,ulint h_long_size){

		assert(log2_max_gap>0);

		return	(ulint(1)<<p)*(log2_max_gap+intlog2(p)) +	//size of hash
				h_long_size*sizeof(triple)*8;					//size of long codes

	}

	//hash table: code prefix -> <decoded value, bit length of the code>
	//if H_len[i]=0, then code exceeds max length (prefix_length). If this
	//is the case, search code in H_long (binary search)

	packed_view<vector> H_val;
	packed_view<vector> H_len;

	//store here all codes longer than prefix_length bits
	//vector of triples <left-shifted Huffman code, gap value, code bit-length>
	vector<std::tuple<ulint,ulint,uint8_t> > H_long;

	std::map<ulint,vector<bool> > encoding;

	//length of codes' prefixes that are indexed in the hash table H
	uint8_t prefix_length=0;

	//max gap length
	ulint log2_max_gap=0;

};

}//namespace bwtil

#endif /* CGAP_DICTIONARY_H_ */
