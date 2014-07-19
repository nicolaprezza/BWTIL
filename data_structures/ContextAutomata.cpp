/*
 * ContextAutomata.cpp
 *
 *  Created on: Jul 14, 2014
 *      Author: nicola
 */

#include "ContextAutomata.h"

namespace bwtil {

ContextAutomata::ContextAutomata(uint k, BackwardFileReader * bfr, bool verbose){

	init(bfr, verbose);
	build(k, bfr, verbose);

}

ContextAutomata::ContextAutomata(BackwardFileReader * bfr, uint overhead, bool verbose){

	init(bfr, verbose);

	//detect optimal k and build automata

	if(verbose) cout << "\n Allowed memory overhead for the automata = " << overhead << "%" << endl;
	if(verbose) cout << " Detecting optimal k ... " << endl;

	uint _k = optimalK(overhead, bfr, verbose);

	if(verbose) cout << " Done. Optimal k = " << _k << endl;

	build( _k , bfr, verbose);

}

ContextAutomata::ContextAutomata(BackwardFileReader * bfr, bool verbose){

	init(bfr, verbose);

	//detect optimal k and build automata
	//default 5% of overhead

	if(verbose) cout << "\n Allowed memory overhead for the automata = 5%" << endl;
	if(verbose) cout << " Detecting optimal k ... " << endl;

	uint _k = optimalK(5, bfr, verbose);

	if(verbose) cout << " Done. Optimal k = " << _k << endl;

	build( _k , bfr, verbose);

}

/*
 * return true with probability p
 */
bool flip_coin(double p){

	return ((double)rand()/(double)RAND_MAX) <= p;

}

uint ContextAutomata::optimalK(uint overhead, BackwardFileReader * bfr, bool verbose){

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

	ulint pos = n-1;//current position in the text
	while(not bfr->BeginOfFile()){

		if(((n-1)-pos)%B==0){//begin of a block

			if(pos+1 >= B ){//if there are at least B characters to be sampled

				if((n-1)-pos==0 or flip_coin(p)){//randomly decide if sample the block

					for(uint i=0;i<B;i++)	//sample B characters
						sampled_text.push_back( bfr->read() );

				}

			}

		}

		bfr->read();//skip character on text
		pos--;

	}

	bfr->rewind();

	if(verbose)
		cout << "  Sampled text size = " << sampled_text.size() << endl;

	//for each context the automata will memorize:
	// sigma edges
	// sigma partial sum counters: log n * sigma
	// approximate total number of bits for each context: O(sigma * log n)

	uint log_sigma = log2(sigma+1);
	uint log_n = log2(n+1);

	/*
	 * Structures in memory:
	 *
	 * vector<uint > prefix_nr; -> sizeof(uint)*CHAR_BIT /context
	 * vector<vector<uint> > edges;//sigma edges for each (k-1)-mer -> <= sigma*sizeof(uint)*CHAR_BIT + sizeof(ulint)*CHAR_BIT / context
	 * CumulativeCounters * counters; -> sigma*sizeof(uint)*CHAR_BIT + sizeof(ulint)*CHAR_BIT / context
	 * DynamicString ** dynStrings; -> sigma*sizeof(uint)*CHAR_BIT / context
	 * ulint * lengths; -> sizeof(ulint)*CHAR_BIT / context
	 *
	 */

	uint A1 = 3*sizeof(uint)*CHAR_BIT;//extimated number of bits per alphabet symbol per context
	uint A2 =  sizeof(uint)*CHAR_BIT + 3*sizeof(ulint)*CHAR_BIT;//extimated number of bits per context

	uint A3 = 10;//empirical scale factor

	//uint A1 = 1202;
	//uint A2 = 25545;

	ulint bits_per_context = A3*(sigma*A1 + A2);

	if(verbose) cout << "  Extimated number of bits per context: " << bits_per_context << endl;

	ulint nr_of_contexts;

	// we want the highest k such that nr_of_contexts*bits_per_context <= n * (overhead/100)

	uint _k = 1;//start from k=1.
	nr_of_contexts=numberOfContexts( _k, sampled_text);

	if(verbose) cout << "  Number of " << _k << "-mers : " << nr_of_contexts << endl;

	while( _k < log_n and nr_of_contexts * bits_per_context <= (n * overhead)/100 ){

		_k++;

		nr_of_contexts=numberOfContexts( _k, sampled_text);

		if(verbose) cout << "  Number of " << _k << "-mers : " << nr_of_contexts << endl;

	}

	if(_k > 1)//we found the first _k above the threshold, so decrease _k. Minimum _k is 1.
		_k--;

	return _k;

}

/*
 * number of contexts of length k in the sampled text
 */
ulint ContextAutomata::numberOfContexts(uint k, vector<symbol> sampled_text){

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

void ContextAutomata::init(BackwardFileReader * bfr, bool verbose){

	if(verbose) cout << "\n*** Building context automata ***\n\n";

	bwFileReader = bfr;

	n = bwFileReader->length();
	null_ptr = ~((uint)0);

	if(verbose) cout << " Text length is " << n << endl;

	remapping = new uint[256];
	inverse_remapping = new symbol[256];

	for(uint i=0;i<256;i++){
		remapping[i]=empty;
		inverse_remapping[i]=0;
	}

	if(verbose) cout << "\n scanning file to detect alphabet ... " << endl;

	vector<symbol> alphabet = vector<symbol>();

	ulint symbols_read=0;
	vector<bool> inserted = vector<bool>(256,false);

	int perc,last_perc=-1;

	while(not bwFileReader->BeginOfFile()){

		symbol s = bwFileReader->read();

		if(s==0){

			cout << "ERROR while reading file " << bwFileReader->getPath() << " : the file contains a 0x0 byte.\n";
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

	if(verbose) cout << "\n Alphabet = { ";

	for (uint i=0;i<alphabet.size();i++){

		if(remapping[alphabet.at(i)]==empty){//new symbol

			remapping[alphabet.at(i)] = sigma;
			sigma++;

		}

		if(verbose) cout << alphabet.at(i) << ' ';

	}

	if(verbose) cout << "}\n\n";

	if(verbose) cout << " Alphabet (ASCII codes) = { ";

	for (uint i=0;i<alphabet.size();i++)
		if(verbose) cout << (ulint)alphabet.at(i) << ' ';

	if(verbose) cout << "}" << endl;

	TERMINATOR = 0;

	for(uint i=1;i<256;i++)
		if(remapping[i]!=empty)
			inverse_remapping[remapping[i]] = i;

	inverse_remapping[TERMINATOR] = 0;//0 is the terminator appended in the file

	bwFileReader->rewind();

}

void ContextAutomata::build(uint k, BackwardFileReader * bfr, bool verbose){

	this->k=k;
	this->bwFileReader = bfr;

	sigma_pow_k_minus_one = 1;
	for(uint i=0;i<k-1;i++)
		sigma_pow_k_minus_one *= sigma;

	ulint q = n/(log2(n)*log2(n)) + 1;//hash size: n/log^2 n

	vector<set<ulint> > H = vector<set<ulint> >( q,set<ulint>() );//the hash

	if(verbose) cout << "\n detecting k-mers ... " << flush;

	ulint context = (ulint)0;//first context

	H.at(context%q).insert(context);

	while(not bfr->BeginOfFile()){

		context = shift(context, ASCIItoCode(bfr->read()) );

		H.at(context%q).insert(context);

	}

	bfr->rewind();

	if(verbose) cout << "done.\n sorting k-mers ... " << flush;

	vector<ulint> k_mers;

	for(ulint i=0;i<q;i++)
		for (std::set<ulint>::iterator it=H.at(i).begin(); it!=H.at(i).end(); ++it)
			k_mers.push_back(*it);

	std::sort(k_mers.begin(),k_mers.end());

	number_of_k_mers = k_mers.size();

	if(verbose) cout << "done. " << k_mers.size() << " nonempty contexts of length " << k << " (including contexts containing terminator character)"  << endl;

	if(verbose) cout << " building automata edges ... "<< flush;

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
	symbol s;

	while(not bfr->BeginOfFile()){

		s = ASCIItoCode(bfr->read());//read symbol
		context = shift(context, s );//new context

		if( edge(current_state,s) == null_ptr ){//edge does not exist: create it

			//search context position and store new pointer

			setEdge(current_state, s, searchContext(context, k_mers));

		}

		//jump following the edge
		current_state = edge(current_state,s);

	}

	bfr->rewind();

	rewind();//go back to initial state

	if(verbose) cout << "done." << endl;

}

} /* namespace bwtil */
