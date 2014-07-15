/*
 * ContextAutomata.cpp
 *
 *  Created on: Jul 14, 2014
 *      Author: nicola
 */

#include "ContextAutomata.h"

namespace bwtil {

ContextAutomata::ContextAutomata(uint k, BackwardFileReader * bfr, bool verbose){

	this->k=k;
	this->bwFileReader = bfr;

	if(verbose) cout << "\n*** Building context automata ***\n\n";

	init(verbose);

	sigma_pow_k_minus_one = 1;
	for(uint i=0;i<k-1;i++)
		sigma_pow_k_minus_one *= sigma;

	null_ptr = ~((uint)0);

	ulint q = n/(log2(n)*log2(n)) + 1;//hash size: n/log^2 n

	vector<set<ulint> > H = vector<set<ulint> >( q,set<ulint>() );//the hash

	if(verbose) cout << "\n detecting k-mers ... ";

	ulint context = (ulint)0;//first context

	H.at(context%q).insert(context);

	while(not bfr->BeginOfFile()){

		context = shift(context, ASCIItoCode(bfr->read()) );

		H.at(context%q).insert(context);

	}

	bfr->rewind();

	if(verbose) cout << "done.\n sorting k-mers ... ";

	vector<ulint> k_mers;

	for(ulint i=0;i<q;i++)
		for (std::set<ulint>::iterator it=H.at(i).begin(); it!=H.at(i).end(); ++it)
			k_mers.push_back(*it);

	std::sort(k_mers.begin(),k_mers.end());

	number_of_k_mers = k_mers.size();

	if(verbose) cout << "done. " << k_mers.size() << " nonempty contexts of length " << k << " (including contexts containing terminator character)"  << endl;

	if(verbose) cout << " building automata edges ... ";

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

	if(verbose) cout << "done.\n";

}

void ContextAutomata::init(bool verbose){

	n = bwFileReader->length();

	if(verbose) cout << " Text length is " << n << endl;

	remapping = new uint[256];
	inverse_remapping = new symbol[256];

	for(uint i=0;i<256;i++){
		remapping[i]=empty;
		inverse_remapping[i]=0;
	}

	if(verbose) cout << "\n scanning file to detect alphabet ... \n";

	vector<symbol> alphabet = vector<symbol>();

	ulint symbols_read=0;
	vector<bool> inserted = vector<bool>(256,false);

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

		symbols_read++;

		if(symbols_read%5000000==0 and verbose)
			cout << " " << 100*((double)symbols_read/(double)n) << "% done.\n";


	}

	if(verbose) cout << " done.\n\n Sorting alphabet ... ";

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

	if(verbose) cout << "}\n";

	TERMINATOR = 0;

	for(uint i=1;i<256;i++)
		if(remapping[i]!=empty)
			inverse_remapping[remapping[i]] = i;

	inverse_remapping[TERMINATOR] = 0;//0 is the terminator appended in the file

	bwFileReader->rewind();

}

} /* namespace bwtil */
