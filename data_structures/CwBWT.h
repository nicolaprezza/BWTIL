/*
 * CwBWT.h
 *
 *  Created on: Jun 24, 2014
 *      Author: nicola
 *
 *      Description: given the path of a file, builds BWT efficiently and in compressed space.
 *
 *      WARNING: text must not contain a 0x0 byte, since this byte is appended as text terminator and included in the BWT
 */

#ifndef CWBWT_H_
#define CWBWT_H_

#include "../common/common.h"
#include "CumulativeCounters.h"
#include "DynamicString.h"
#include "BackwardFileReader.h"

namespace bwtil {

class CwBWT {

public:

	class CwBWTIterator{

	public:

		CwBWTIterator(CwBWT * bwt);

		symbol next();
		bool hasNext(){return position<n;};

	private:

		CwBWT * bwt;
		ulint context;//pointer to context of next symbol
		ulint prefix;//pointer to prefix of the next context (if prefix>=prefix size then we are in the suffix)
		ulint suffix;//pointer to the suffix of the next context

		ulint n;

		ulint position;//position in the bwt of the next char to be returned

	};

	CwBWT(){};
	CwBWT(string path, bool verbose=true);//creates CwBWT with default number of contexts ( O(n/(log^3 n)) )
	CwBWT(string path, uint k, bool verbose=true);//creates CwBWT with desired context length

	CwBWTIterator getIterator(){return CwBWTIterator(this);};

	void toFile(string path);//save BWT to file
	symbol * toArray();//returns BWT as a char array of size n+1

	string toString();

	double empiricalEntropy(){return Hk;};//empirical entropy of order k, computed with actual observed frequencies.
	double actualEntropy(){return bits_per_symbol;};//actual entropy of order k obtained with the Huffman compressor. Always >= empiricalEntropy()

	ulint size(){return n+1;};//size of text + terminator character

	void debug();

protected:

	void appendToPrefix(ulint context, symbol s);
	ulint prefixLength(ulint context);
	symbol prefixAt(ulint context,ulint i);
	void printRSSstat();

	ulint number_of_contexts;//equals sigma^k

	ulint terminator_position;

	symbol * inverse_remapping;//from symbol -> to char (file)
	symbol TERMINATOR;//equal to sigma

	ulint n;//length of the text (without text terminator)

private:

	void computeEmpiricalEntropy();//empirical entropy of order k, computed with actual observed frequencies.
	void computeActualEntropy();//actual entropy of order k obtained with the Huffman compressor. Always >= empiricalEntropy()

	void build(bool verbose);//build CwBWT (after all structures have been created)
	void init1(string path, bool verbose);
	void init2(string path, bool verbose);

	ulint shift(ulint context, symbol s);

	void printContext(ulint x);

	static const uint empty = 256;

	uint k;//context length and order of compression (entropy H_k). default: k = ceil( log_sigma(n/log^3 n) )
	uint sigma;

	BackwardFileReader bwFileReader;

	//structure for each context block:
	CumulativeCounters * counters;
	DynamicString ** dynStrings;
	vector<ulint> * frequencies;//frequencies[i] = frequency of each symbol in {0,...,sigma-1} in the context i

	vector<symbol> ** prefixes;//each context can have a (small) prefix

	ulint * lengths;//length of each context

	uint * remapping;//from char (file) -> to symbols in {0,...,sigma-1}

	ulint sigma_pow_k_minus_one;//sigma^(k-1)

	double Hk,bits_per_symbol;

};

} /* namespace bwtil */
#endif /* CwBWT_H_ */
