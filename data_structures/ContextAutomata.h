/*
 * ContextAutomata.h
 *
 *  Created on: Jul 14, 2014
 *      Author: nicola
 */

#ifndef CONTEXTAUTOMATA_H_
#define CONTEXTAUTOMATA_H_

#include "../common/common.h"
#include "BackwardFileReader.h"
#include "CumulativeCounters.h"
#include "DynamicString.h"

namespace bwtil {

class ContextAutomata {

public:

	ContextAutomata(){};

	/*
	 * ASSUMPTION: alphabet is {0,...,sigma-1}, where 0 is the terminator character (appearing only at the end of file)
	 */
	ContextAutomata(uint k, BackwardFileReader * bfr, bool verbose = false);

	//overhead : k will be chosen such that the automata will require approximately space n*overhead/100 bits, where n is the text length
	ContextAutomata(BackwardFileReader * bfr, uint overhead, bool verbose = false);

	ContextAutomata(BackwardFileReader * bfr, bool verbose = false);//default 5% of overhead

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

	void init(BackwardFileReader * bfr, bool verbose);
	void build(uint k, BackwardFileReader * bfr, bool verbose);
	uint optimalK(uint overhead, BackwardFileReader * bfr,bool verbose);
	ulint numberOfContexts(uint k, vector<symbol> sampled_text);

	static const uint empty = 256;

	symbol TERMINATOR;//0x0

	BackwardFileReader * bwFileReader;

	symbol * inverse_remapping;//from symbol -> to char (file)
	uint * remapping;//from char (file) -> to symbols in {0,...,sigma-1}

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
