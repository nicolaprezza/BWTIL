/*
 * CwBWT.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: nicola
 */

#include "CwBWT.h"
#include "../extern/getRSS.c"

namespace bwtil {

CwBWT::CwBWT(string path, bool verbose){

	//detect optimal k
	//BackwardFileReader bfr(path);
	//bfr.close();

	k = 2;//TODO

	init(path, k, verbose);

}

CwBWT::CwBWT(string path, uint k, bool verbose){//creates CwBWT with desired number of contexts

	init(path,k,verbose);

}

void CwBWT::init(string path, uint k, bool verbose){

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

	ca = ContextAutomata(k, &bwFileReader, true);

	this->k = k;
	number_of_contexts = ca.numberOfStates();

	sigma = ca.alphabetSize();//this takes into account also the terminator character
	TERMINATOR = 0;

	initStructures(path, verbose);

	build(verbose);

	if(verbose) printRSSstat();

}

void CwBWT::initStructures(string path, bool verbose){

	frequencies = new vector<ulint>[number_of_contexts];
	for(ulint i=0;i<number_of_contexts;i++)
		frequencies[i] = vector<ulint>(sigma,0);

	lengths = new ulint[number_of_contexts];

	for(ulint i=0;i<number_of_contexts;i++)
		lengths[i]=0;

	symbol s;

	ulint symbols_read=0;

	if(verbose) cout << "\n*** Scanning input file to compute context frequencies ***" << endl;

	while(not bwFileReader.BeginOfFile()){

		s = ca.ASCIItoCode( bwFileReader.read() );//this symbol has as context the current state of the automata

		lengths[ ca.currentState() ]++;//new symbol in this context:increment

		frequencies[ ca.currentState() ].at(s) = frequencies[ ca.currentState() ].at(s)+1;//increment the frequency of s in the context

		ca.goTo(s);

		symbols_read++;

		if(symbols_read%5000000==0 and verbose)
			cout << " " << 100*((double)symbols_read/(double)n) << "% done." << endl;

	}

	lengths[ ca.currentState() ]++;//first context in the text: will contain only terminator
	frequencies[ ca.currentState() ].at(0)++;//terminator

	computeEmpiricalEntropy();

	if(verbose) cout << " Done.\n" << endl;

	ulint max_len=0;
	for(ulint i=0;i<number_of_contexts;i++)
		if(lengths[i]>max_len)
			max_len=lengths[i];

	if(verbose) cout << " Largest context has " << max_len << " characters" << endl;

	if(verbose) cout << "\n*** Creating data structures (dynamic compressed strings) ***";

	dynStrings = new DynamicString*[number_of_contexts];
	for(ulint i=0;i<number_of_contexts;i++){

		dynStrings[i] = new DynamicString(&frequencies[i]);

	}

	delete [] frequencies;

	computeActualEntropy();

	counters = new CumulativeCounters[number_of_contexts];
	for(ulint i=0;i<number_of_contexts;i++)
		counters[i] = CumulativeCounters(sigma,lengths[i]);

	delete [] lengths;

	if(verbose) cout << "\n\n Done." << endl;

	if(verbose){

		cout << "\n k-th order empirical entropy of the text is " << empiricalEntropy() << endl;
		cout << " bits per symbol used (only compressed text): " << actualEntropy() << endl;

	}

}

int compare(uint i, uint j, vector<uint> sorted_positions, symbol * last_symbols, uint k){

	if(i==j) return 0;

	if(last_symbols[i]<last_symbols[j])
		return -1;
	else if (last_symbols[i]>last_symbols[j])
		return 1;

	//last_symbols[i]==last_symbols[j]

	i++;
	j++;

	if(i==k) return -1;

	if(j==k) return 1;

	for(uint l=0;l<sorted_positions.size();l++){

		if(sorted_positions.at(l)==i) return -1;
		if(sorted_positions.at(l)==j) return 1;

	}

	return 0;

}

void CwBWT::build(bool verbose){

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

	if(verbose) cout << "\n*** Main cw-bwt algorithm (context-wise incremental construction of the BWT) *** " << endl;

	uint perc,last_percentage=1;

	while(not bwFileReader.BeginOfFile()){

		perc = 10*(uint)(10*((double)(n-pos)/(double)n));

		if(perc>last_percentage and verbose){
			cout << perc << "% done." << endl;
			last_percentage = perc;
		}

		if((n-pos)%5000000==0 and verbose)
			cout << 100*((double)(n-pos)/(double)n) << "% done." << endl;

		head = ca.ASCIItoCode( bwFileReader.read() );//this symbol has context corresponding to ca.currentState(). symbol entering from left in context
		tail = context_char[pos%k];// = (pos+k)%k . Symbol exiting from right of the context

		context_char[pos%k] = head;//buffer head symbol, overwriting the symbol exiting from tail of the context

		ca.goTo(head);
		new_terminator_context = ca.currentState();

		//substitute the terminator with the symbol head (coordinates terminator_context,terminator_pos)

		counters[new_terminator_context].increment(tail);
		new_terminator_pos = counters[new_terminator_context].getCount(tail) +  dynStrings[terminator_context]->rank(head,terminator_pos);

		dynStrings[terminator_context]->insert(head,terminator_pos);

		//update terminator coordinates

		terminator_context = new_terminator_context;
		terminator_pos = new_terminator_pos;

		pos--;

	}

	dynStrings[terminator_context]->insert(TERMINATOR,terminator_pos);//insert the terminator character

	bwFileReader.close();//close input file

	if(verbose) cout << "Done." << endl;
}

void CwBWT::printRSSstat(){

	size_t peakSize = getPeakRSS( );
	cout << "\nPeak RAM usage (Bytes): " <<  peakSize <<endl;
	cout << "Bits per symbol used (all structures in RAM): " <<  ((double)peakSize/(double)size())*8 <<endl;

}

void CwBWT::computeEmpiricalEntropy(){

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

void CwBWT::computeActualEntropy(){//actual entropy of order k obtained with the Huffman compressor. Always >= empiricalEntropy()

	//warning:to be called AFTER initialization of structures

	bits_per_symbol = 0;//k-order empirical entropy

	for(ulint i=0;i<number_of_contexts;i++){//for each non-empty context

		if(lengths[i]>0){

			double H0 = dynStrings[i]->entropy();

			bits_per_symbol += H0*((double)lengths[i]/(double)n);

		}
	}

}

string CwBWT::toString(){

	CwBWTIterator it = getIterator();

	string s = "";

	symbol c;

	while(it.hasNext()){
		c = it.next();

		if(c==0) c = '#';//since 0x0 byte cannot be visualized, in the string version it is printed as #

		s += c;
	}

	return s;

}

symbol * CwBWT::toArray(){//returns CwBWT as a char array of size n+1

	symbol * bwt = new symbol[n+1];

	CwBWTIterator it = getIterator();

	ulint i=0;
	while(it.hasNext()){

		bwt[i] = it.next();
		i++;

	}

	return bwt;

}

void CwBWT::toFile(string path){//save CwBWT to file

	FILE *fp;

	if ((fp = fopen(path.c_str(), "wb")) == NULL) {
		VERBOSE_CHANNEL<< "Cannot open file " << path << endl;
		exit(1);
	}

	CwBWTIterator it = getIterator();

	ulint i=0;
	symbol c;

	if(n<100) cout << "saving bwt : ";

	while(it.hasNext()){

		c = it.next();
		fwrite(&c, sizeof(symbol), 1, fp);

		if(n<100){  if(c==0)cout << "#"; else cout << c; }

		i++;

	}

	if(n<100) cout<< endl;

	fclose(fp);

}

void CwBWT::debug(){

	for(uint i=0;i<number_of_contexts;i++)
		cout << dynStrings[i]->toString() << endl;

}


CwBWT::CwBWTIterator::CwBWTIterator(CwBWT * bwt){

	this->bwt = bwt;

	context=0;

	while(context < bwt->number_of_contexts and bwt->dynStrings[context]->size()==0)//search nonempty context
		context++;

	i=0;

	n = bwt->n+1;//add text terminator

	position = 0;

}

symbol CwBWT::CwBWTIterator::next(){

	if(not hasNext())
		return 0;

	symbol s = bwt->dynStrings[context]->access(i);

	i++;

	if(i>=bwt->dynStrings[context]->size()){//out of suffix: search new nonempty context

		context++;

		while(context < bwt->number_of_contexts and bwt->dynStrings[context]->size()==0)//search nonempty context
			context++;

		i=0;

	}

	position++;

	return bwt->ca.CodeToASCII(s);

}



} /* namespace compressed_bwt_construction */
