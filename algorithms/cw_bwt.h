/*
 * cw_bwt.h
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
#include "../data_structures/PartialSums.h"
#include "../data_structures/DynamicString.h"
#include "../data_structures/BackwardFileReader.h"
#include "../data_structures/ContextAutomata.h"
#include "../extern/getRSS.h"

namespace bwtil {

class cw_bwt {

public:

	class cw_bwt_iterator{

	public:

		cw_bwt_iterator(cw_bwt * bwt){

			this->bwt = bwt;

			context=0;

			while(context < bwt->number_of_contexts and bwt->dynStrings[context].size()==0)//search nonempty context
				context++;

			i=0;

			n = bwt->n+1;//add text terminator

			position = 0;

		}

		symbol next(){

			if(not hasNext())
				return 0;

			symbol s = bwt->dynStrings[context].access(i);

			i++;

			if(i>=bwt->dynStrings[context].size()){//out of suffix: search new nonempty context

				context++;

				while(context < bwt->number_of_contexts and bwt->dynStrings[context].size()==0)//search nonempty context
					context++;

				i=0;

			}

			position++;

			return bwt->ca.CodeToASCII(s);

		}

		bool hasNext(){return position<n;};

	private:

		cw_bwt * bwt;
		ulint context;//pointer to context of next symbol
		ulint i;//next position to be read in the current context

		ulint n;

		ulint position;//position in the bwt of the next char to be returned

	};

	cw_bwt(){};

	//creates cw_bwt with default number of contexts ( O(n/(log^3 n)) )
	cw_bwt(string path, bool verbose=true){

		bwFileReader = BackwardFileReader(path);
		n = bwFileReader.length();

		ca = ContextAutomata(&bwFileReader, 5, true);//Default automata overhead
		k = ca.contextLength();

		init(path, verbose);

	}

	//creates cw_bwt with desired context length k
	cw_bwt(string path, uint k, bool verbose){

		this->k = k;

		if(k==0){
			cout << "Error: context length must be k>0" << endl;
			exit(0);
		}

		if(verbose) cout << "\nContext length is k = " << k << endl;

		bwFileReader = BackwardFileReader(path);
		n = bwFileReader.length();

		if(n<=k){
			cout << "Error: File length n must be n>k, where k is the context length." << endl;
			exit(0);
		}

		uint log_n = log2(n);

		if(k>=log_n){
			cout << "Error: k is too large. k must be <= log_s(n), where s is the alphabet size." << endl;
			exit(0);
		}

		ca = ContextAutomata(k, &bwFileReader, true);

		init(path, verbose);

	}

	string toString(){

		cw_bwt_iterator it = getIterator();

		string s = "";

		symbol c;

		while(it.hasNext()){
			c = it.next();

			if(c==0) c = '#';//since 0x0 byte cannot be visualized, in the string version it is printed as #

			s += c;
		}

		return s;

	}

	symbol * toArray(){//returns cw_bwt as a char array of size n+1

		symbol * bwt = new symbol[n+1];

		cw_bwt_iterator it = getIterator();

		ulint i=0;
		while(it.hasNext()){

			bwt[i] = it.next();
			i++;

		}

		return bwt;

	}

	void toFile(string path,bool verbose=true){//save cw_bwt to file

		FILE *fp;

		if ((fp = fopen(path.c_str(), "wb")) == NULL) {
			VERBOSE_CHANNEL<< "Cannot open file " << path << endl;
			exit(1);
		}

		cw_bwt_iterator it = getIterator();

		ulint i=0;
		symbol c;

		int perc,last_perc=-1;

		while(it.hasNext()){

			c = it.next();
			fwrite(&c, sizeof(symbol), 1, fp);

			i++;

			perc = (100*i)/n;
			if(verbose and perc>last_perc and perc%5==0){
				cout << " " << perc << "% Done." << endl;
				last_perc=perc;
			}

		}

		fclose(fp);

	}

	void debug(){

	}

	cw_bwt_iterator getIterator(){return cw_bwt_iterator(this);};

	double empiricalEntropy(){return Hk;};//empirical entropy of order k, computed with actual observed frequencies.
	double actualEntropy(){return bits_per_symbol;};//actual entropy of order k obtained with the Huffman compressor. Always >= empiricalEntropy()

	ulint size(){return n+1;};//size of text + terminator character

protected:

	void printRSSstat(){

		size_t peakSize = getPeakRSS( );

		if(peakSize/((ulint)1<<30) > 0)
			cout << "\nPeak RAM usage: " <<  (double)peakSize/((ulint)1<<30) << " GB" <<endl;
		else if (peakSize/((ulint)1<<20) > 0)
			cout << "\nPeak RAM usage: " <<  (double)peakSize/((ulint)1<<20) << " MB" <<endl;
		else if (peakSize/((ulint)1<<10) > 0)
			cout << "\nPeak RAM usage: " <<  (double)peakSize/((ulint)1<<10) << " KB" <<endl;

		cout << "Bits per symbol used (all structures in RAM): " <<  ((double)peakSize/(double)size())*8 <<endl;
		cout << "Bytes per symbol used (all structures in RAM): " <<  ((double)peakSize/(double)size()) <<endl;

	}

	ulint number_of_contexts;

	ulint terminator_position;

	symbol TERMINATOR;//equal to 0

	ulint n;//length of the text (without text terminator)

	ContextAutomata ca;

private:

	void computeEmpiricalEntropy(){

		//warning:to be called AFTER initialization of structures

		Hk = 0;//k-order empirical entropy

		for(ulint i=0;i<number_of_contexts;i++){//for each non-empty context

			if(lengths[i]>0){

				double H0=0;//0-order entropy of this context

				for(uint s=0;s<sigma;s++){//for each symbol in the alphabet

					double f = (double)frequencies[i].at(s)/(double)lengths[i];

					if(f>0)
						H0 += -f*log2(f);

				}

				Hk += H0*((double)lengths[i]/(double)n);

			}
		}

	}

	void computeActualEntropy(){//actual entropy of order k obtained with the Huffman compressor. Always >= empiricalEntropy()

		//warning:to be called AFTER initialization of structures

		bits_per_symbol = 0;//k-order empirical entropy

		for(ulint i=0;i<number_of_contexts;i++){//for each non-empty context

			if(lengths[i]>0){

				double H0 = dynStrings[i].entropy();

				bits_per_symbol += H0*((double)lengths[i]/(double)n);

			}
		}

	}

	void init(string path, bool verbose){

		number_of_contexts = ca.numberOfStates();

		sigma = ca.alphabetSize();//this takes into account also the terminator character
		TERMINATOR = 0;

		initStructures(path, verbose);

		build(verbose);

		ulint sum_of_heights=0;
		ulint sum_of_lenghts=0;

		for(ulint i=0;i<number_of_contexts;i++){

			sum_of_lenghts += dynStrings[i].numberOfBits();
			sum_of_heights += dynStrings[i].sumOfHeights();

		}

		double avg_height = (double)sum_of_heights/(double)sum_of_lenghts;

		if(verbose) cout << "\nAverage packed B-tree height is: " << avg_height << endl;

		if(verbose) printRSSstat();

	}

	void initStructures(string path, bool verbose){

		frequencies = new vector<ulint>[number_of_contexts];
		for(ulint i=0;i<number_of_contexts;i++)
			frequencies[i] = vector<ulint>(sigma,0);

		lengths = new ulint[number_of_contexts];

		for(ulint i=0;i<number_of_contexts;i++)
			lengths[i]=0;

		symbol s;

		ulint symbols_read=0;

		if(verbose) cout << "\n*** Scanning input file to compute context frequencies ***" << endl << endl;

		int perc,last_perc=-1;

		while(not bwFileReader.BeginOfFile()){

			s = ca.ASCIItoCode( bwFileReader.read() );//this symbol has as context the current state of the automata

			lengths[ ca.currentState() ]++;//new symbol in this context:increment

			frequencies[ ca.currentState() ].at(s) = frequencies[ ca.currentState() ].at(s)+1;//increment the frequency of s in the context

			ca.goTo(s);

			perc = (100*symbols_read)/n;

			if(perc>last_perc and (perc%5)==0 and verbose){
				cout << " " << perc << "% done." << endl;
				last_perc=perc;
			}
			symbols_read++;

		}

		lengths[ ca.currentState() ]++;//first context in the text: will contain only terminator
		frequencies[ ca.currentState() ].at(0)++;//terminator

		computeEmpiricalEntropy();

		if(verbose) cout << " Done.\n" << endl;

		//99 intervals [i,i+100M/100] and one final interval i>100M

		//statistics:
		uint number_of_intervals = 20;
		ulint max_len = (10*n)/number_of_contexts;
		ulint step = max_len / number_of_intervals;
		ulint tot_intervals = number_of_intervals+1;

		vector<ulint> stats = vector<ulint>(tot_intervals,0);

		ulint max=0;
		for(ulint i=0;i<number_of_contexts;i++)
			if(lengths[i]>max)
				max=lengths[i];

		for(ulint i=0;i<number_of_contexts;i++){

			if(lengths[i]>=max_len)
				stats.at(tot_intervals-1) = stats.at(tot_intervals-1)+1;
			else{

				uint pos=lengths[i]/step;
				stats.at(pos) = stats.at(pos)+1;

			}

		}

		ulint exp_context_length = n/number_of_contexts;

		if(verbose) cout << " Largest context has " << max << " characters" << endl;
		if(verbose) cout << " Expected context size (if uniform text) is " << exp_context_length << " characters" << endl;

		//compute expected packed B-tree height

		{

			bitv sample_bv(256,exp_context_length);

			uint nr_of_leaves = sample_bv.info().leaves;
			uint b = sample_bv.info().buffer; //worst-case tree fanout
			if(b<=1) b=2;
			double exp_height = (log2(nr_of_leaves)/log2(b));

			if(verbose) cout << " Expected worst-case packed B-tree height (if uniform text) is " << exp_height << endl << endl;

		}

		if(verbose) cout << " Context length statistics: " << endl;

		if(verbose){

			for(uint i=0;i<tot_intervals-1;i++)
				cout << " [" << i*step << ", " << (i+1)*step << "[ -> " << stats.at(i)<<endl;

			cout << " [ " << max_len <<", inf [ -> " << stats.at(tot_intervals-1)<<endl;

		}


		if(verbose) cout << "\n*** Creating data structures (dynamic compressed strings) ***" << endl << endl;

		perc=0;
		last_perc=-1;

		dynStrings = vector<dynamic_string_t >(number_of_contexts);
		for(ulint i=0;i<number_of_contexts;i++){

			dynStrings[i] = dynamic_string_t(&frequencies[i]);
			frequencies[i].clear();//free memory

			perc = (100*i)/number_of_contexts;

			if(perc>last_perc and (perc%10)==0 and verbose){
				cout << " " << perc << "% done." << endl;
				last_perc=perc;
			}

		}

		delete [] frequencies;

		computeActualEntropy();

		partial_sums = new PartialSums[number_of_contexts];
		for(ulint i=0;i<number_of_contexts;i++)
			partial_sums[i] = PartialSums(sigma,lengths[i]);

		delete [] lengths;

		if(verbose){

			cout << "\n k-th order empirical entropy of the text is " << empiricalEntropy() << endl;
			cout << " bits per symbol used (only compressed text): " << actualEntropy() << endl;

		}

		if(verbose) cout << "\nData structures created." << endl;

	}

	void build(bool verbose){

		ulint pos = n-1;//current position on text (char to be inserted in the bwt)
		ulint terminator_context,terminator_pos, new_terminator_context,new_terminator_pos;//coordinates of the terminator character

		ca.rewind();//go back to first state
		bwFileReader.rewind();

		//context of length k before position n (excluded):
		terminator_context = ca.currentState();//context
		symbol * context_char = new symbol[k];//context in char format

		for(uint i=0;i<k;i++)
			context_char[i] = 0;

		//memorize position of the terminator
		terminator_pos = 0;

		//now start main algorithm

		symbol head,tail;//head=symbol to be inserted, tail=symbol exiting from the context

		if(verbose) cout << "\n*** Main cw-bwt algorithm (context-wise incremental construction of the BWT) *** " << endl << endl;

		int perc,last_percentage=-1;

		while(not bwFileReader.BeginOfFile()){

			perc = (100*(n-pos-1))/n;

			if((perc%5==0) and (perc>last_percentage) and verbose){
				cout << " " << perc << "% done." << endl;
				last_percentage = perc;
			}

			head = ca.ASCIItoCode( bwFileReader.read() );//this symbol has context corresponding to ca.currentState(). symbol entering from left in context
			tail = context_char[pos%k];// = (pos+k)%k . Symbol exiting from right of the context

			context_char[pos%k] = head;//buffer head symbol, overwriting the symbol exiting from tail of the context

			ca.goTo(head);
			new_terminator_context = ca.currentState();

			//substitute the terminator with the symbol head (coordinates terminator_context,terminator_pos)

			partial_sums[new_terminator_context].increment(tail);
			new_terminator_pos = partial_sums[new_terminator_context].getCount(tail) +  dynStrings[terminator_context].rank(head,terminator_pos);

			dynStrings[terminator_context].insert(head,terminator_pos);

			//update terminator coordinates

			terminator_context = new_terminator_context;
			terminator_pos = new_terminator_pos;

			pos--;

		}

		dynStrings[terminator_context].insert(TERMINATOR,terminator_pos);//insert the terminator character

		bwFileReader.close();//close input file

		delete [] context_char;

		if(verbose) cout << " Done." << endl;
	}

	static const uint empty = 256;

	uint k;//context length and order of compression (entropy H_k). default: k = ceil( log_sigma(n/log^3 n) )
	uint sigma;

	BackwardFileReader bwFileReader;

	//structure for each context block:
	PartialSums * partial_sums;
	vector<dynamic_string_t> dynStrings;
	vector<ulint> * frequencies;//frequencies[i] = frequency of each symbol in {0,...,sigma-1} in the context i

	ulint * lengths;//length of each context

	double Hk,bits_per_symbol;

};

} /* namespace bwtil */
#endif /* CWBWT_H_ */
