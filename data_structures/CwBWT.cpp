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

	init1(path, verbose);

	int _k = floor( log2( n/(log2(n)*log2(n)*log2(n)) )/log2(sigma) );

	if(_k<=0)
		k = 1;
	else
		k = _k;

	if(verbose) cout << "\nContext length is k = " << k << endl;

	if(n<=k){
		cout << "Error: File length n must be n>k, where k is the context length.\n";
		exit(0);
	}

	number_of_contexts = 1;//sigma^k
	for(uint i=0;i<(uint)k;i++)
		number_of_contexts *= sigma;

	if(verbose) cout << "\nNumber of contexts is " << number_of_contexts << endl;

	init2(path, verbose);

	build(verbose);

	if(verbose) printRSSstat();

}

CwBWT::CwBWT(string path, uint k, bool verbose){//creates CwBWT with desired number of contexts

	init1(path, verbose);

	if(k==0){
		cout << "Error: context length must be k>0\n";
		exit(0);
	}

	this->k=k;

	if(verbose) cout << "\nContext length is k = " << k << endl;

	if(n<=k){
		cout << "Error: File length n must be n>k, where k is the context length.\n";
		exit(0);
	}

	number_of_contexts = 1;//sigma^k
	for(uint i=0;i<k;i++)
		number_of_contexts *= sigma;

	if(verbose) cout << "\nNumber of contexts is " << number_of_contexts << endl;

	init2(path, verbose);

	build(verbose);

	bwFileReader.close();

	if(verbose) printRSSstat();

}

void CwBWT::init1(string path, bool verbose){

	bwFileReader = BackwardFileReader(path);

	n = bwFileReader.length();

	if(verbose) cout << "Text length is " << n << endl;

	remapping = new uint[256];
	inverse_remapping = new symbol[256];

	for(uint i=0;i<256;i++){
		remapping[i]=empty;
		inverse_remapping[i]=0;
	}

	if(verbose) cout << "\nStep 1/3: scanning file to detect alphabet ... \n";

	vector<symbol> alphabet = vector<symbol>();

	ulint symbols_read=0;
	vector<bool> inserted = vector<bool>(256,false);

	while(not bwFileReader.BeginOfFile()){

		symbol s = bwFileReader.read();

		if(s==0){

			cout << "ERROR while reading file " << path << " : the file contains a 0x0 byte.\n";
			exit(0);

		}

		if(not inserted.at(s)){
			inserted.at(s) = true;
			alphabet.push_back(s);
		}

		symbols_read++;

		if(symbols_read%5000000==0 and verbose)
			cout << 100*((double)symbols_read/(double)n) << "% done.\n";


	}

	if(verbose) cout << "Done.\n\nSorting alphabet ... ";

	std::sort(alphabet.begin(),alphabet.end());

	if(verbose) cout << "Done. Alphabet size: sigma = " << alphabet.size() << endl;

	sigma = 0;

	if(verbose) cout << "Alphabet = { ";

	for (uint i=0;i<alphabet.size();i++){

		if(remapping[alphabet.at(i)]==empty){//new symbol

			remapping[alphabet.at(i)] = sigma;
			sigma++;

		}

		if(verbose) cout << alphabet.at(i) << ' ';

	}

	if(verbose) cout << "}\n\n";

	if(verbose) cout << "Alphabet (ASCII codes) = { ";

	for (uint i=0;i<alphabet.size();i++)
		if(verbose) cout << (ulint)alphabet.at(i) << ' ';

	if(verbose) cout << "}\n";

	TERMINATOR = sigma;

	for(uint i=1;i<256;i++)
		if(remapping[i]!=empty)
			inverse_remapping[remapping[i]] = i;

	inverse_remapping[TERMINATOR] = 0;//0 is the terminator appended in the file

}

void CwBWT::init2(string path, bool verbose){

	sigma_pow_k_minus_one = 1;
	for(uint i=0;i<k-1;i++)
		sigma_pow_k_minus_one *= sigma;

	prefixes = new vector<symbol>*[number_of_contexts];
	for(ulint i=0;i<number_of_contexts;i++)
		prefixes[i] = NULL;

	frequencies = new vector<ulint>[number_of_contexts];
	for(ulint i=0;i<number_of_contexts;i++)
		frequencies[i] = vector<ulint>(sigma,0);

	lengths = new ulint[number_of_contexts];

	for(ulint i=0;i<number_of_contexts;i++)
		lengths[i]=0;

	bwFileReader.rewind();

	//compute first context of length k
	ulint context = 0;

	symbol * ctx = new symbol[k];

	for(uint i=0;i<k;i++)
		ctx[i] = remapping[bwFileReader.read()];

	for(uint i=0;i<k;i++)
		context = context*sigma + ctx[k-i-1];

	ulint last_context=context;

	delete [] ctx;

	symbol s;

	ulint symbols_read=k;

	if(verbose) cout << "\nStep 2/3: scanning input file to compute context frequencies\n";

	while(not bwFileReader.BeginOfFile()){

		s = remapping[bwFileReader.read()];//this symbol has context 'context'

		lengths[context]++;//new symbol in this context:increment

		frequencies[context].at(s) = frequencies[context].at(s)+1;//increment the frequency of s in the context

		context = shift(context,s);//update context with s for the next symbol (if any)

		symbols_read++;

		if(symbols_read%5000000==0 and verbose)
			cout << 100*((double)symbols_read/(double)n) << "% done.\n";

	}

	lengths[context]++;//first context in the text: will contain only terminator
	frequencies[context].at(0)++;//fake frequencies in first context

	computeEmpiricalEntropy();

	if(verbose) cout << "Done.\n\n";

	ulint nr_nonempty_contexts=0;
	for(ulint i=0;i<number_of_contexts;i++)
		if(lengths[i]>0)
			nr_nonempty_contexts++;

	if(verbose) cout << nr_nonempty_contexts << " nonempty contexts.\n";

	ulint max_len=0;
	for(ulint i=0;i<number_of_contexts;i++)
		if(lengths[i]>max_len)
			max_len=lengths[i];

	if(verbose) cout << "Largest context has " << max_len << " characters\n";

	if(verbose) cout << "\nCreating data structures (dynamic compressed strings) ... ";

	dynStrings = new DynamicString*[number_of_contexts];
	for(ulint i=0;i<number_of_contexts;i++){

		dynStrings[i] = DynamicString::getDynamicString(&frequencies[i]);

	}

	delete [] frequencies;

	computeActualEntropy();

	counters = new CumulativeCounters[number_of_contexts];
	for(ulint i=0;i<number_of_contexts;i++)
		counters[i] = CumulativeCounters(sigma,lengths[i]);

	counters[last_context].setBaseCounter();

	delete [] lengths;

	if(verbose) cout << "Done.\n";

	if(verbose){

		cout << "\nk-th order empirical entropy of the text is " << empiricalEntropy() << endl;
		cout << "bits per symbol used (only compressed text): " << actualEntropy() << endl;

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

	ulint pos = n-k-1;//current position on text (char to be inserted in the bwt)
	ulint terminator_context,terminator_pos, new_terminator_context,new_terminator_pos;//coordinates of the terminator character

	//context of length k before position n (excluded):
	terminator_context = 0;//context in integer format
	symbol * context_char = new symbol[k];//context in char format

	bwFileReader.rewind();

	//compute last context
	for(uint i=0;i<k;i++)
		context_char[(n-i-1)%k] = remapping[bwFileReader.read()];

	for(uint i=n-k;i<n;i++)
		terminator_context = terminator_context*sigma + context_char[i%k];

	//memorize position of the terminator
	terminator_pos = 0;

	//now start main algorithm

	symbol head,tail;//head=symbol to be inserted, tail=symbol exiting from the context

	if(verbose) cout << "\nStep 3/3: main algorithm (building incrementally the CwBWT)\n";

	//first of all, insert the last k characters of the text

	symbol * last_symbols = new symbol[k];
	ulint * last_contexts = new ulint[k];

	vector<uint> sorted_positions;//indexes of the previous vectors sorted lexicographically

	for(uint i=0;i<k;i++)
		last_contexts[i]=0;

	for(uint i=0;i<k;i++)
		last_symbols[i] = context_char[(n-k+i)%k];

	//compute contexts (padding with 0)
	for(uint i=0;i<k;i++)
		for(uint j=0;j<k;j++)
			last_contexts[i] = last_contexts[i]*sigma + (n-k+1+i+j>=n?0:context_char[(n-k+1+i+j)%k]);

	//sort

	for(uint i=0;i<k;i++){//insert symbol in position k-i-1 (array last_symbols)

		uint to_be_inserted = k-i-1;
		uint pos=0;

		for(uint j=0;j<sorted_positions.size();j++){//find position where to insert the element

			if(compare(to_be_inserted, sorted_positions.at(j), sorted_positions, last_symbols, k) < 0) break;

			pos++;

		}

		sorted_positions.insert(sorted_positions.begin() + pos,to_be_inserted);

	}

	for(uint i=0;i<k;i++){//insert last k characters in decreasing lex order (since they are appended to the prefixes)

		uint pos = k-i-1;
		appendToPrefix(last_contexts[sorted_positions.at(pos)],last_symbols[sorted_positions.at(pos)]);

	}

	delete [] last_contexts;
	delete [] last_symbols;

	uint perc,last_percentage=1;

	while(not bwFileReader.BeginOfFile()){

		perc = 10*(uint)(10*((double)(n-pos)/(double)n));

		if(perc>last_percentage and verbose){
			cout << perc << "% done.\n";
			last_percentage = perc;
		}

		if((n-pos)%5000000==0 and verbose)
			cout << 100*((double)(n-pos)/(double)n) << "% done.\n";

		head = remapping[bwFileReader.read()];//this symbol has context 'context_int'
		tail = context_char[pos%k];// = (pos+k)%k

		context_char[pos%k] = head;//buffer head symbol, overwriting the symbol exiting from tail of the context

		new_terminator_context = shift(terminator_context,head);

		//substitute the terminator with the symbol head (coordinates terminator_context,terminator_pos)

		counters[new_terminator_context].increment(tail);
		new_terminator_pos = counters[new_terminator_context].getCount(tail) +  dynStrings[terminator_context]->rank(head,terminator_pos);

		dynStrings[terminator_context]->insert(head,terminator_pos);

		//update terminator coordinates

		terminator_context = new_terminator_context;
		terminator_pos = new_terminator_pos;

		pos--;

	}

	ulint count=0;
	for(ulint i=0;i<number_of_contexts;i++){

		count += prefixLength(i);

		if(i==terminator_context){
			terminator_position = count + terminator_pos;
			break;
		}

		count += dynStrings[i]->size();

	}

	//cout << "final terminator position = " << terminator_position<<endl;

	bwFileReader.close();//close input file

	if(verbose) cout << "Done.\n";
}

void CwBWT::printRSSstat(){

	size_t peakSize = getPeakRSS( );
	cout << "\nPeak RAM usage (Bytes): " <<  peakSize <<endl;
	cout << "Bits per symbol used (all structures in RAM): " <<  ((double)peakSize/(double)size())*8 <<endl;

}

void CwBWT::appendToPrefix(ulint context, symbol s){

	if(prefixes[context]==NULL)
		prefixes[context] = new vector<symbol>();

	prefixes[context]->push_back(s);

}

ulint CwBWT::prefixLength(ulint context){

	if(prefixes[context]==NULL)
		return 0;

	return prefixes[context]->size();

}
symbol CwBWT::prefixAt(ulint context,ulint i){

#ifdef DEBUG
	if(prefixes[context]==NULL or i>=prefixes[context]->size()){
		cout << "ERROR (CwBWT.cpp): accessing null prefix.\n";
		exit(0);
	}
#endif

	return prefixes[context]->at(prefixes[context]->size()-i-1);

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

ulint CwBWT::shift(ulint context, symbol s){

	return (context - (context%sigma))/sigma + ((ulint)s)*sigma_pow_k_minus_one;

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

void CwBWT::printContext(ulint x){

	string s;

	for(uint i=0;i<k;i++){

		s += inverse_remapping[x%sigma];
		x = (x-(x%sigma))/sigma;

	}

	cout << endl;
	for(uint i=0;i<k;i++)
		cout << s.at(k-i-1);
	cout << endl;

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

	while(it.hasNext()){

		c = it.next();
		fwrite(&c, sizeof(symbol), 1, fp);

		i++;

	}

	fclose(fp);

}

void CwBWT::debug(){

	for(uint i=0;i<number_of_contexts;i++)
		cout << dynStrings[i]->toString() << endl;

}

CwBWT::CwBWTIterator::CwBWTIterator(CwBWT * bwt){

	this->bwt = bwt;

	context=0;

	while(context < bwt->number_of_contexts and bwt->prefixLength(context)==0 and bwt->dynStrings[context]->size()==0)//search nonempty context
		context++;

	prefix=0;
	suffix=0;

	n = bwt->n;

	position = 0;

}

symbol CwBWT::CwBWTIterator::next(){

	symbol s = 0;

	if(position==bwt->terminator_position){

		s = bwt->inverse_remapping[bwt->TERMINATOR];
		position++;
		return s;

	}

	if(prefix < bwt->prefixLength(context)){//we are in the prefix

		s = bwt->prefixAt(context,prefix);

		prefix++;

		if(prefix >= bwt->prefixLength(context)){//out of prefix

			if(bwt->dynStrings[context]->size() == 0){//no dynamic string: search next nonempty context

				context++;

				while(context < bwt->number_of_contexts and bwt->prefixLength(context)==0 and bwt->dynStrings[context]->size()==0)//search nonempty context
					context++;

				prefix=0;
				suffix=0;

			}

		}

	}else{//we are in the suffix

		s = bwt->dynStrings[context]->access(suffix);

		suffix++;

		if(suffix>=bwt->dynStrings[context]->size()){//out of suffix: search new nonempty context

			context++;

			while(context < bwt->number_of_contexts and bwt->prefixLength(context)==0 and bwt->dynStrings[context]->size()==0)//search nonempty context
				context++;

			prefix=0;
			suffix=0;

		}

	}

	position++;

	return bwt->inverse_remapping[s];

}



} /* namespace compressed_bwt_construction */
