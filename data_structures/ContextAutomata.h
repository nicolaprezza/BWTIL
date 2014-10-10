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

/*
 * ContextAutomata.h
 *
 *  Created on: Jul 14, 2014
 *      Author: nicola
 */

#ifndef CONTEXTAUTOMATA_H_
#define CONTEXTAUTOMATA_H_

#include "../common/common.h"
#include "BackwardFileIterator.h"
#include "../data_structures/BackwardStringIterator.h"
#include "PartialSums.h"
#include "DynamicString.h"

namespace bwtil {

class ContextAutomata {

public:

	ContextAutomata(){};

	//ASSUMPTION: alphabet is {0,...,sigma-1}, where 0 is the terminator character (appearing only at the end of file)
	ContextAutomata(uint k, BackwardIterator * bfr, bool verbose){

		init(bfr, verbose);
		build(k, bfr, verbose);

	}

	//overhead : k will be chosen such that the automata will require approximately space n*overhead/100 bits, where n is the text length
	ContextAutomata(BackwardIterator * bfr, uint overhead, bool verbose){
		init(bfr, verbose);

		//detect optimal k and build automata

		if(verbose) cout << "\n Allowed memory overhead for the automata = " << overhead << "%" << endl;
		if(verbose) cout << " Detecting optimal k ... " << endl;

		uint _k = optimalK(overhead, bfr, verbose);

		if(verbose) cout << " Done. Optimal k = " << _k << endl;

		build( _k , bfr, verbose);

	}

	//default 5% of overhead
	ContextAutomata(BackwardIterator * bfr, bool verbose){

		init(bfr, verbose);

		//detect optimal k and build automata
		//default 5% of overhead

		if(verbose) cout << "\n Allowed memory overhead for the automata = 5%" << endl;
		if(verbose) cout << " Detecting optimal k ... " << endl;

		uint _k = optimalK(5, bfr, verbose);

		if(verbose) cout << " Done. Optimal k = " << _k << endl;

		build( _k , bfr, verbose);

	}

	//jump from current state following the edge labeled with s. WARNING: alphabet must be {0,...,sigma-1}
	void goTo(symbol s){

		current_state = edge(current_state, s);

		if(current_state==null_ptr){
			cout << "ERROR (ContextAutomata) : using non-initialized edge.\n";
			exit(0);
		}

	};

	void goToASCII(symbol s){

		current_state = edge(current_state, ASCIItoCode(s) );

		if(current_state==null_ptr){
			cout << "ERROR (ContextAutomata) : using non-initialized edge.\n";
			exit(0);
		}

	};

	ulint currentState(){return current_state;};//return current state number
	ulint numberOfStates(){return number_of_k_mers;};

	void rewind(){current_state=0;};//return to initial state

	symbol ASCIItoCode(symbol c){return remapping[c];}
	symbol CodeToASCII(symbol c){return inverse_remapping[c];}

	symbol ASCIItoCodeNoTerminator(symbol c){if(c==0) return 0;  return remapping[c]-1;}
	symbol CodeToASCIINoTerminator(symbol c){return inverse_remapping[c+1];}

	uint alphabetSize(){return sigma;}//included terminator

	ulint textLength(){return n;};

	uint contextLength(){return k;}

private:

	//return true with probability p
	bool flip_coin(double p){

		return ((double)rand()/(double)RAND_MAX) <= p;

	}

	uint optimalK(uint overhead, BackwardIterator * bfr, bool verbose){

		//strategy: sample randomly n/log n characters of the text (in contiguous blocks of size B)
		//and try k=1,... until suitable k is found. There will be at most log_sigma n iterations, so work is linear.

		ulint B = 1000; //number of characters per block

		if(n<B)
			B = n;

		vector<symbol> sampled_text;

		srand(time(NULL));

		ulint nr_of_blocks = n/B;//total number of blocks in the text
		ulint sampled_n = n/(uint)log2(n);//expected size of sampled text
		ulint nr_of_sampled_blocks = sampled_n/B;//expected number of sampled blocks
		double p = (double)nr_of_sampled_blocks/(double)nr_of_blocks;//probability that a block is sampled

		int perc,last_perc=-1;
		if(verbose)
			cout << "  Sampling text ... " << endl;

		ulint pos = n-1;//current position in the text
		while(not bfr->begin()){

			if(((n-1)-pos)%B==0){

				if(pos+1 >= B ){//if there are at least B characters to be sampled

					if((n-1)-pos==0 or flip_coin(p)){//randomly decide if sample the block

						for(uint i=0;i<B;i++)	//sample B characters
							sampled_text.push_back( bfr->read() );

					}

				}

				perc = (100*((n-1)-pos))/n;

				if(perc>last_perc and (perc%5)==0 and verbose){
					cout << "  " << perc << "% done." << endl;
					last_perc=perc;
				}

			}

			bfr->read();//skip character on text
			pos--;

		}

		bfr->rewind();

		if(verbose)
			cout << "\n  Sampled text size = " << sampled_text.size() << endl;

		/*
		 * Extimate number of bits in memory for each context:
		 * O(sigma * log n) bits for each context.
		 *
		 * Structures in memory:
		 *
		 * vector<uint > prefix_nr;
		 * vector<vector<uint> > edges;
		 * CumulativeCounters * counters;
		 * DynamicString ** dynStrings;
		 * ulint * lengths;
		 *
		 */

		PartialSums sample_cumulative_counter = PartialSums(sigma,n);//sample of cumulative counter to extimate its memory consumption
		DynamicString<bitv> sample_dynstring = DynamicString<bitv>(vector<ulint>(sigma,1));

		ulint bits_per_k_mer =
				CHAR_BIT * sizeof(DynamicString<bitv> *) + //pointers to DynamicStrings
				sample_cumulative_counter.bitSize()  + //cumulative counters
				sample_dynstring.bitSize() + //DynamicStrings
				CHAR_BIT*sizeof(vector<uint >) + //vector prefix_nr
				CHAR_BIT*sizeof(uint) + //content of vector prefix_nr
				CHAR_BIT*sizeof(ulint); //lengths

		ulint bits_per_k_1_mer =
				CHAR_BIT*sizeof(vector<uint>) + //vector edges
				CHAR_BIT*sigma*sizeof(uint); //content of vector edges

		uint log_n = log2(n+1);

		if(verbose) cout << "  Extimated number of bits per k-mer: " << bits_per_k_mer << endl;
		if(verbose) cout << "  Extimated number of bits per (k-1)-mer: " << bits_per_k_1_mer << endl;

		ulint nr_of_k_mers;
		ulint nr_of_k_1_mers=1;//number of (k-1)-mers

		// we want the highest k such that nr_of_contexts*bits_per_context <= n * (overhead/100)

		uint _k = 1;//start from k=1.
		nr_of_k_mers=numberOfContexts( _k, sampled_text);

		if(verbose) cout << "  Number of " << _k << "-mers : " << nr_of_k_mers << endl;

		while( _k < log_n and (nr_of_k_mers * bits_per_k_mer + nr_of_k_1_mers * bits_per_k_1_mer <= (n * overhead)/100) ){

			_k++;

			nr_of_k_1_mers = nr_of_k_mers;
			nr_of_k_mers=numberOfContexts( _k, sampled_text);

			if(verbose) cout << "  Number of " << _k << "-mers : " << nr_of_k_mers << endl;

		}

		if(_k > 1)//we found the first _k above the threshold, so decrease _k.
			_k--;
		else
			_k = 1;//minimum k is 1

		return _k;

	}

	/*
	 * number of contexts of length k in the sampled text
	 */
	ulint numberOfContexts(uint k, vector<symbol> sampled_text){

		sigma_pow_k_minus_one = 1;
		for(uint i=0;i<k-1;i++)
			sigma_pow_k_minus_one *= sigma;

		ulint q = n/(log2(n)*log2(n)) + 1;//hash size: n/log^2 n

		vector<set<ulint> > H = vector<set<ulint> >( q,set<ulint>() );//the hash

		ulint context = (ulint)0;//first context

		H.at(context%q).insert(context);

		for(ulint i=0;i<sampled_text.size();i++){

			context = shift(context, ASCIItoCode(sampled_text.at(i)) );

			H.at(context%q).insert(context);

		}

		vector<ulint> k_mers;

		for(ulint i=0;i<q;i++)
			for (std::set<ulint>::iterator it=H.at(i).begin(); it!=H.at(i).end(); ++it)
				k_mers.push_back(*it);

		return k_mers.size();

	}

	void init(BackwardIterator * bfr, bool verbose){

		if(verbose) cout << "\n*** Building context automata ***\n\n";

		bwIt = bfr;

		n = bwIt->length();
		null_ptr = ~((uint)0);

		if(verbose) cout << " Text length is " << n << endl;

		remapping = vector<uint>(256);
		inverse_remapping = vector<symbol>(256);

		for(uint i=0;i<256;i++){
			remapping[i]=empty;
			inverse_remapping[i]=0;
		}

		if(verbose) cout << "\n scanning file to detect alphabet ... " << endl;

		vector<symbol> alphabet = vector<symbol>();

		ulint symbols_read=0;
		vector<bool> inserted = vector<bool>(256,false);

		int perc,last_perc=-1;

		while(not bwIt->begin()){

			symbol s = bwIt->read();

			if(s==0){

				cout << "ERROR while reading input text : the text contains a 0x0 byte.\n";
				exit(0);

			}

			if(not inserted.at(s)){
				inserted.at(s) = true;
				alphabet.push_back(s);
			}

			perc = (100*symbols_read)/n;

			if(perc>last_perc and (perc%5)==0 and verbose){
				cout << " " << perc << "% done." << endl;
				last_perc=perc;
			}

			symbols_read++;

		}

		if(verbose) cout << " done.\n\n Sorting alphabet ... " << flush;

		std::sort(alphabet.begin(),alphabet.end());

		if(verbose) cout << "done. Alphabet size: sigma = " << alphabet.size() << endl;

		sigma = 1;//code 0x0 is for the terminator

		//if(verbose) cout << "\n Alphabet = { ";

		for (uint i=0;i<alphabet.size();i++){

			if(remapping[alphabet.at(i)]==empty){//new symbol

				remapping[alphabet.at(i)] = sigma;
				sigma++;

			}

			//if(verbose) cout << alphabet.at(i) << ' ';

		}

		//if(verbose) cout << "}\n\n";

		if(verbose) cout << "\n Alphabet (ASCII codes) = { ";

		for (uint i=0;i<alphabet.size();i++)
			if(verbose) cout << (ulint)alphabet.at(i) << ' ';

		if(verbose) cout << "}" << endl;

		TERMINATOR = 0;

		for(uint i=1;i<256;i++)
			if(remapping[i]!=empty)
				inverse_remapping[remapping[i]] = i;

		inverse_remapping[TERMINATOR] = 0;//0 is the terminator appended in the file

		bwIt->rewind();

	}

	void build(uint k, BackwardIterator * bfr, bool verbose){

		this->k=k;
		this->bwIt = bfr;

		sigma_pow_k_minus_one = 1;
		for(uint i=0;i<k-1;i++)
			sigma_pow_k_minus_one *= sigma;

		ulint q = n/(log2(n)*log2(n)) + 1;//hash size: n/log^2 n

		vector<set<ulint> > H = vector<set<ulint> >( q,set<ulint>() );//the hash

		if(verbose) cout << "\n detecting k-mers ... " << endl;

		ulint context = (ulint)0;//first context

		H.at(context%q).insert(context);

		int perc,last_perc=-1;
		ulint symbols_read=0;

		while(not bfr->begin()){

			context = shift(context, ASCIItoCode(bfr->read()) );

			H.at(context%q).insert(context);

			perc = (100*symbols_read)/n;

			if(perc>last_perc and (perc%5)==0 and verbose){
				cout << " " << perc << "% done." << endl;
				last_perc=perc;
			}

			symbols_read++;

		}

		bfr->rewind();

		if(verbose) cout << " done.\n\n sorting k-mers ... " << flush;

		vector<ulint> k_mers;

		for(ulint i=0;i<q;i++)
			for (std::set<ulint>::iterator it=H.at(i).begin(); it!=H.at(i).end(); ++it)
				k_mers.push_back(*it);

		std::sort(k_mers.begin(),k_mers.end());

		number_of_k_mers = k_mers.size();

		if(verbose) cout << "done. " << k_mers.size() << " nonempty contexts of length k = " << k << " (including contexts containing terminator character)"  << endl;

		if(verbose) cout << " building automata edges ... "<< endl;

		uint nr_of_prefixes=0;
		prefix_nr.push_back(nr_of_prefixes);

		for(uint i=1;i<number_of_k_mers;i++){

			if( prefix(k_mers.at(i)) == prefix(k_mers.at(i-1)) )
				prefix_nr.push_back(nr_of_prefixes);
			else{

				nr_of_prefixes++;
				prefix_nr.push_back(nr_of_prefixes);

			}

		}

		nr_of_prefixes++;

		edges = vector<vector<uint> >(nr_of_prefixes, vector<uint>(sigma,null_ptr)  );

		context = (ulint)0;//first context
		current_state = 0;

		perc=0;
		last_perc=-1;
		symbols_read=0;

		ulint context_from;
		ulint context_to;

		for(ulint i = 0;i<number_of_k_mers;i++ ){//for each k-mer

			context_from = k_mers.at(i);

			for(symbol s = 0;s<sigma;s++){//for each symbol

				if( edge(i,s) == null_ptr ){//edge does not exist: create it

					context_to = shift(context_from, s );

					//search context position and store new pointer

					setEdge(i, s, searchContext(context_to, k_mers));

				}

			}

			perc = (100*i)/number_of_k_mers;
			if(verbose and perc>last_perc and perc%10==0){

				last_perc=perc;
				cout << " " << perc << "% Done." << endl;

			}

		}

		rewind();//go back to initial state

		if(verbose) cout << " done." << endl;
		if(verbose) cout << "\nContext automata completed." << endl;

	}

	static const uint empty = 256;

	symbol TERMINATOR;//0x0

	BackwardIterator * bwIt;

	vector<symbol> inverse_remapping;//from symbol -> to char (file)
	vector<uint> remapping;//from char (file) -> to symbols in {0,...,sigma-1}

	ulint current_state;
	uint null_ptr;

	vector<uint > prefix_nr;//for each k_mer, address of its prefix in the array edges
	vector<vector<uint> > edges;//sigma edges for each (k-1)-mer

	ulint prefix(ulint context){ return (context - (context%sigma))/sigma; }
	ulint shift(ulint context, symbol s){ return prefix(context) + ((ulint)s)*sigma_pow_k_minus_one;	}

	uint edge(uint state, symbol s){ return edges.at( prefix_nr.at(state) ).at(s); }
	void setEdge(uint state, symbol s, uint value){ edges.at( prefix_nr.at(state) ).at(s) = value; }
	uint searchContext(ulint context, vector<ulint> k_mers){ return std::lower_bound(k_mers.begin(),k_mers.end(),context) - k_mers.begin(); }

	ulint number_of_k_mers;

	uint sigma;//alphabet size
	uint k;//context length
	ulint sigma_pow_k_minus_one;
	ulint n;

};

} /* namespace bwtil */
#endif /* CONTEXTAUTOMATA_H_ */
