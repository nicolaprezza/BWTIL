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
#include "../common/common.h"
#include <map>

namespace bwtil{

class cgap_dictionary{

	template<typename T>
	class bin_tree{

	public:

		class node{

		public:

			node(){};

			node(vector<vector<bool> > bitvectors, vector<T> values){

				assert(bitvectors.size()>0);
				assert(bitvectors.size()==values.size());

				if(bitvectors.size()==1){

					assert(bitvectors[0].size()==0);
					value=values[0];

				}else{

					vector<vector<bool> > bv0;
					vector<vector<bool> > bv1;

					vector<T> values0;
					vector<T> values1;

					for(ulint i = 0; i<bitvectors.size(); ++i){

						if(bitvectors[i][0]){

							if(bitvectors[i].size()>1)
								bv1.push_back( {bitvectors[i].begin()+1,bitvectors[i].end()} );
							else
								bv1.push_back(vector<bool>());

							assert(bv1[bv1.size()-1].size()==0 or bv1[bv1.size()-1].size()==bitvectors[i].size()-1 );

							values1.push_back(values[i]);

						}else{

							if(bitvectors[i].size()>1)
								bv0.push_back( {bitvectors[i].begin()+1,bitvectors[i].end()} );
							else
								bv0.push_back(vector<bool>());

							assert(bv0[bv0.size()-1].size()==0 or bv0[bv0.size()-1].size()==bitvectors[i].size()-1 );

							values0.push_back(values[i]);

						}

					}

					children.push_back({bv0,values0});
					children.push_back({bv1,values1});

				}

			}

			bool isLeaf(){return children.size()==0;}

			T getValue(){
				assert(isLeaf());
				return value;
			}

			node getChild0(){
				assert(children.size()==2);
				return children[0];
			}
			node getChild1(){
				assert(children.size()==2);
				return children[1];
			}

		private:

			vector<node> children;
			T value=0;

		};

		bin_tree(){}

		//build a binary tree with these bitvectors
		//in the leafs: <value, leaf depth>
		bin_tree(vector<vector<bool> > bitvectors, vector<T> values){

			assert(bitvectors.size()==values.size());
			root = {bitvectors,values};

		}

		//input: bitvector encoded as a 64-bits unsigned int, stored in the leftmost bits
		//output: descend the tree according to the read bits, return <value stored in the leaf, leaf depth>
		pair<T,T> search(ulint bitv){

			return search(bitv,root,0);

		}

	private:

		//input: bitvector encoded as a 64-bits unsigned int, stored in the leftmost bits
		//input: i = start position in bitv (leftmost)
		//input: start node (default: root)
		//output: descend the tree according to the read bits, return <value stored in the leaf, leaf depth>
		pair<T,T> search(ulint bitv,node n, T i){

			assert(i<=sizeof(ulint)*8);
			assert( i!=sizeof(ulint)*8 or n.isLeaf());

			if(n.isLeaf())
				return {n.getValue(),i};

			uint shift = (sizeof(ulint)*8-1)-i;
			bool bit = (bitv>>shift) & ulint(1);

			return search(bitv,bit?n.getChild1():n.getChild0(),i+1);

		}

		node root;

	};

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

		for(auto g : gaps){

			assert(g.first>0);
			assert(g.second>0);

			u += (g.first*g.second);
			n += g.second;

		}

		//compute optimal prefix length for the hash table.
		//we choose prefix_length = log2( n/(log u*log n) ), so total size of the hash is o(n)

		if(log2(u)*log2(n)>=n)
			prefix_length = 8;
		else
			prefix_length = (uint8_t)ceil(log2( (double)n/(log2(u)*log2(n)) ));

		//we index at least 2^8 = 256 bits
		if(prefix_length<8)
			prefix_length=8;

		assert(prefix_length<64);

		//build Huffman tree and retrieve boolean codes
		vector<vector<bool> > codes;

		{
			vector<ulint> freqs;

			for(auto g : gaps)
				freqs.push_back(g.second);

			HuffmanTree<ulint> ht(freqs);

			codes = ht.getCodes();

		}

		assert(codes.size()==gaps.size());

		//build the code function
		for(ulint i=0;i<codes.size();++i)
			encoding[gaps[i].first] = codes[i];

		ulint H_size = ulint(1)<<prefix_length;

		H = vector<pair<ulint,ulint> >(H_size,{0,0});

		exceeds = vector<bool>(H.size());

		//build hash

		//temporary vector of containers of bitvectors. In the end, each container will be
		//converted to a binary tree
		vector<vector<pair<vector<bool>,ulint > > > temp_bitv;
		temp_bitv.push_back({});//position 0 is reserved, so we just insert an empty element

		for(ulint i=0;i<gaps.size();++i){

			//code length
			auto l = codes[i].size();

			if(l<=prefix_length){

				//the hash is sufficient to decode this code. Compute H entry
				ulint h = 0;

				for(auto b : codes[i])
					h = h*2+b;

				//shift to left so to occupy exactly prefix_length bits
				h = h << (prefix_length-l);

				//fill all combinations of h followed by any bit sequence (in total prefix_length bits)
				for(ulint j=0;j<(ulint(1)<<(prefix_length-l));++j){
					exceeds[h+j] = false;
					H[h+j] = {gaps[i].first,l};
				}

			}else{

				//compute hash using first prefix_length bits
				ulint h = 0;

				for(uint j=0;j<prefix_length;++j)
					h = h*2 + codes[i][j];

				exceeds[h] = true;

				//we have to create the container of bitvectors
				if(H[h].second==0){

					H[h].second = temp_bitv.size();

					//create new empty container
					temp_bitv.push_back({});
					temp_bitv[temp_bitv.size()-1].push_back(
							{
								{codes[i].begin()+prefix_length,codes[i].end()},
								gaps[i].first
							});


				}else{

					//use the container already existent
					temp_bitv[ H[h].second ].push_back(
							{
								{codes[i].begin()+prefix_length,codes[i].end()},
								gaps[i].first
							});

				}

			}

		}

		//now convert containers of bitvector to binary trees

		for(ulint i=1;i<temp_bitv.size();++i){

			auto b = temp_bitv[i];
			//b is a vector of pairs <bitvector, value>
			//separate the 2 components

			vector<vector<bool> > bitv;
			vector<ulint> values;

			assert(b.size()>0);

			for(auto p : b){

				bitv.push_back(p.first);
				values.push_back(p.second);

			}

			assert(bitv.size()==values.size());
			partial_htrees.push_back({bitv,values});

		}

		//decrease by 1 all H pointers to partial trees
		for(ulint i=0;i<H.size();++i){

			if(exceeds[i]){

				assert(H[i].second>0);
				H[i].second--;

			}

		}

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

		if(exceeds[h]){

			//suffix of the bitvector: search it in the appropriate tree
			ulint s = x << prefix_length;
			auto p = partial_htrees[H[h].second].search(s);

			return {p.first,p.second+prefix_length};

		}

		//does not exceed: just access H
		return H[h];

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

	vector<bool > encode(ulint g){
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

		return gaps;

	}

private:

	//hash table: code prefix -> <decoded value, bit length of the code>
	//if exceeds[i]=true, then instead of bit length there is a pointer to a partial
	//Huffman tree (vector partial_htrees)
	vector<pair<ulint,ulint> > H;

	//exceeds[i] = true iif code i is ambiguous, i.e. need to read more bits
	vector <bool> exceeds;

	vector<bin_tree<ulint> > partial_htrees;

	std::map<ulint,vector<bool> > encoding;

	//length of codes' prefixes that are indexed in the hash table H
	uint8_t prefix_length=0;


};

}//namespace bwtil

#endif /* CGAP_DICTIONARY_H_ */
