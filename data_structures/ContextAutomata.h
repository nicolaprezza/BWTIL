/*
 * ContextAutomata.h
 *
 *  Created on: Jul 14, 2014
 *      Author: nicola
 */

#ifndef CONTEXTAUTOMATA_H_
#define CONTEXTAUTOMATA_H_

#include "../common/common.h"

namespace bwtil {

class ContextAutomata {

public:

	/*
	 * ASSUMPTION: alphabet is {0,...,sigma-1}, where 0 is the terminator character (appearing only at the end of file)
	 */
	ContextAutomata(uint sigma,uint k);

	//jump from current state following the edge labeled with s. WARNING: alphabet must be {0,...,sigma-1}
	//if edge is nonexistent, create new edge and new state
	void goTo(symbol s);

	ulint currentState(){return current_state;};//return current state number
	ulint numberOfStates();

	void rewind();//return to initial state

private:

	ulint current_state;

	vector<vector<uint> > edges;//sigma edges for each k-mer

	uint null_ptr;

	ulint shift(ulint context, symbol s){ return (context - (context%sigma))/sigma + ((ulint)s)*sigma_pow_k_minus_one;	}

	uint sigma;//alphabet size
	uint k;//context length
	ulint sigma_pow_k_minus_one;

	symbol tail;//symbol

};

} /* namespace bwtil */
#endif /* CONTEXTAUTOMATA_H_ */
