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

//============================================================================
// Name        : LZ77.h
// Author      : Nicola Prezza
// Version     : 1.0
// Description : In this version, just count the number of LZ77 phrases of a text.

/*
 * This class is obsolete. Use lz77_parser instead.
 *
 * Counts number of phrases of the LZ77 parse. Uses a dynamic compressed BWT.
 * zero-order compressed space, n log n time.
 *
 */
//============================================================================

#ifndef LZ77_H_
#define LZ77_H_

#include "../common/common.h"
#include "DynamicBWT.h"
#include "dynamic_vector.h"
#include "FileReader.h"


namespace bwtil {

template <typename bitvector_type>
class LZ77 {

public:

	typedef DynamicBWT<bitvector_type, dynamic_vector<bitvector_type> > dynamic_bwt_type;

	LZ77(){}

	/*
	 * given the path of a ASCII-coded file, builds the LZ77 parse. Warning: the text file must contain < 256 distinct characters (one is reserved for the BWT terminator)
	 */

	/*	v1 : LZ77 variant 1: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase Wc is inserted in the dictionary. With this variant, the last token may contain the terminator 0x0
	*	v2 : LZ77 variant 2: when extending the current phrase W with a character c, if Wc does not occur previously, a new phrase W is inserted in the dictionary, and c is part of the next phrase. With this variant, the last token never contains the terminator 0x0.
	*/
	enum variant {v1,v2};

	/*
	 * token: an element of the parse
	 */
	struct token{

		string phrase;
		ulint start_position;
		//phrases composed only by the first occurrence of a character in the text do not have a defined start position
		bool start_position_is_defined;

	};

	struct options{

		input_mode mode=text;
		bool verbose=false;//output percentages
		variant lz_variant=v1;
		ulint block=0;//output number of phrases every block characters. If 0, don't print anything.
		symbol sep=0;//separator: output number of phrases each time sep is seen in the stream. These characters are ignored in phrase computation. If 0, there are no separators.
		bool prepend_alphabet=false;//add a prefix 'a_1a_2...a_k' to the text, where {a_1, ..., a_k} is the alphabet

	};

	/*
	 * build parse of the input file. Note that file should be terminated by a character that does not appear elsewhere in order
	 * to have a well formed parse.
	 *
	 * With lz77 version 2, the file should moreover start with all alphabet characters concatenated
	 *
	 */
	LZ77(options opt, string input){

		this->opt = opt;
		this->input = input;
		alphabet_iterator=1;//start from 1 since terminator char is present

		symbol_to_int = vector<uint>(256);
		sigma=0;

		if(opt.verbose) cout << "Scanning input to detect alphabet ... " << endl;

		//if input string is a file path, then open file
		if(opt.mode==file_path)
			fr = FileReader(input);

		//create alphabet
		set<symbol> alphabet_set;

		if(opt.mode==file_path){

			while(not fr.eof()){
				symbol s = fr.get();
				if(s==TERMINATOR){

					cout << "Error: input text contains a 0x0 character, which is reserved."<<endl;
					exit(1);

				}
				alphabet_set.insert(s);
			}

			fr.rewind();

		}else{

			for(ulint i=0;i<input.size();++i){

				symbol s = input[i];
				if(s==TERMINATOR){

					cout << "Error: input text contains a 0x0 character, which is reserved."<<endl;
					exit(1);

				}
				alphabet_set.insert(s);

			}

		}

		//insert terminator in alphabet
		alphabet_set.insert(TERMINATOR);

		alphabet = vector<symbol>(alphabet_set.size());

		std::copy(alphabet_set.begin(), alphabet_set.end(), alphabet.begin());

		//compute character remapping
		for(ulint i=0;i<alphabet.size();i++){

			symbol_to_int[alphabet[i]] = sigma;
			sigma++;

		}

		assert(sigma<256);

		if(opt.verbose) cout << "Scanning input to compute character frequencies ... " << endl;

		//compute character frequencies
		vector<ulint> freqs(sigma,0);
		while(not eof())
			freqs[ symbol_to_int[get()] ]++;

		rewind();
		number_of_phrases=0;

		dbwt = dynamic_bwt_type(freqs, 0);
		interval = pair<ulint, ulint>(0,dbwt.size());//current interval

		current_phrase=string();

		if(opt.verbose) cout << "Now parsing ... " << endl;

	}

	token get_token(){

		number_of_phrases++;

		if(opt.block>0 and global_position%opt.block==0)
			cout << global_position << "\t" << number_of_phrases << endl;//print number of phrases

		if(end_of_parse()){

			cout << "END" << endl;

			if(not file_closed and opt.mode == file_path){
				fr.close();
				file_closed=true;
			}

			//if parse is terminated, return a terminator token
			return {string()+char(TERMINATOR),0,false};

		}

		//if mode is v1, initial interval must be <0,n>
		assert( opt.lz_variant==v2 or (interval.second==dbwt.size() and interval.first==0));

		//if mode is v2 and interval is empty, then we have a phrase of 1 character

		if(opt.lz_variant==v2 and interval.second<=interval.first){

			assert(current_phrase.size()==1);

			string current_phrase_copy(current_phrase);
			current_phrase = string();
			interval = pair<ulint, ulint>(0,dbwt.size());

			return {current_phrase_copy, 0, false};

		}

		ulint low_interval=0;
		symbol s=0;

		//repeat until we do not have a match
		while(interval.second>interval.first){

			//read symbol from stream
			s = get();

			//if character is a separator, output number of phrases and skip all separators
			if(opt.sep>0 and s==opt.sep){

				while((s=get())==opt.sep){}//skip all separators
				cout << global_position << "\t" << number_of_phrases << endl;//print number of phrases

			}

			//store low part of the interval
			low_interval=interval.first;
			//extend search with the new character
			interval = dbwt.BS( interval, symbol_to_int[s] );

			//if new interval is not empty, then s is part of the current phrase
			if(interval.second>interval.first){

				//extend with s

				current_phrase+=char(s);

				//add s to the BWT
				dbwt.extend( symbol_to_int[s] );
				//increment upper bound because we inserted a new prefix that falls in this interval
				interval.second++;

			}

		}//while interval not empty

		//at this point, extending with s caused the interval to become empty.

		//phrase w=current_phrase occurs, but ws does not. the following is the location of an occurrence of w:
		//if phrase is empty, no occurrence
		ulint occurrence = 0;

		if(opt.lz_variant==v1){

			//extend BWT with mismatching character
			dbwt.extend( symbol_to_int[s] );
			//reset interval
			interval = pair<ulint, ulint>(0,dbwt.size());

			//variant v1: ws is part of this phrase

			//s is the first occurrence of that character
			if(current_phrase.size()==0){

				//return <'s', -, false> : no start position

				return {string()+char(s), 0, false};

			}

			//else: phrase contains at least 1 character

			string current_phrase_copy=string(current_phrase)+char(s);

			//reset current phrase
			current_phrase=string();

			return {current_phrase_copy, 0, false};

		}else if(opt.lz_variant==v2){

			//variant v2: the phrase to be returned is only w
			//and s is part of the next phrase

			//s is the first occurrence of that character
			if(current_phrase.size()==0){

				//reset interval
				interval = pair<ulint, ulint>(0,dbwt.size());

				//extend the BWT
				dbwt.extend( symbol_to_int[s] );

				//return <'s', -, false> : no start position
				return {string()+char(s), 0, false};

			}

			//else: phrase contains at least 1 character

			//before to extend the BWT, we have to search the mismatching character
			interval = dbwt.BS( pair<ulint, ulint>(0,dbwt.size()), symbol_to_int[s] );
			dbwt.extend( symbol_to_int[s] );

			if(interval.second>interval.first){
				//increment upper bound because we inserted a new prefix that falls in this interval
				interval.second++;
				//else: interval is empty and phrase will be output at the next call of getToken
			}

			string current_phrase_copy(current_phrase);

			//reset current phrase. s is part of the next phrase!
			current_phrase=string()+char(s);

			return {current_phrase_copy, 0, false};

		}

		return{string(),0,false};

	}

	bool end_of_parse(){
		assert(global_position<=size());
		return global_position==size();
	}

	/*
	 * returns total size of the stream, inclulded prefix/suffix
	 */
	ulint size(){

		//if alphabet is added as prefix: size is alphabet size - 1 (i.e. without terminator) plus 1 (the terminator at the end) plus the string length
		//else: size is string length plus 1 (the terminator at the end)
		ulint increment = 	(opt.prepend_alphabet?alphabet.size()-1:0) + 1;

		if(opt.mode==file_path){

			return fr.size()+increment;

		}

		return input.size()+increment;

	}

	/* get the parsed text WITHOUT terminator appended at the end.
	 *
	 */
	string get_text(){

		string text;

		rewind();

		//temporary disable verbose, if enabled.
		bool verbose = opt.verbose;
		opt.verbose = false;

		while(not eof()){

			uchar s = get();
			if(s!=TERMINATOR)
				text += s;

		}

		rewind();

		opt.verbose = verbose;

		return text;

	}

	//ulint getNumberOfPhrases(){return number_of_phrases;}

	/*
	 * output:
	 *
	 * - in lz77 version 1: a list of pairs <wc,i>, where w is a word, c a character, and i an integer.
	 *   meaning: word w occurs in position i. Word wc never occurs before.
	 * - in lz77 version 2: a list of pairs <w, i>, where w is a word, c a character, and i an integer.
	 *   meaning: word w in position i.
	 */
	/*vector<token> getParse(){

		return parse;

	}*/

private:

	symbol get(){

		assert(global_position<size());

		if(opt.verbose){

			int perc=(global_position*100)/size();
			if(perc>=last_perc+5){

				cout << perc << " % done ... " << endl;
				last_perc=perc;

			}

		}

		if(opt.prepend_alphabet and alphabet_iterator<alphabet.size()){

			global_position++;

			return alphabet[alphabet_iterator++];

		}

		if(global_position==size()-1 ){

			global_position++;

			return TERMINATOR;//return terminator character

		}

		if(opt.mode==file_path){

			global_position++;

			symbol s = fr.get();

			return s;

		}

		global_position++;
		return input[position++];

	}

	bool eof(){

		assert(global_position<=size());
		return global_position==size();

	}

	void rewind(){

		last_perc=-1;
		alphabet_iterator=1;//start from 1 if terminator char is present
		global_position=0;
		position=0;

		if(opt.mode==file_path){

			fr.rewind();

		}

	}

	dynamic_bwt_type dbwt;

	//dynamic_bwt_t dbwt;//the dynamic BWT
	pair<ulint, ulint> interval;//current interval

	symbol TERMINATOR=0;

	//remapping of symbols:
	vector<uint> symbol_to_int;
	vector<token> parse;//LZ77 parse: vector of pairs <phrase, start position>
	vector<symbol> alphabet;
	uint alphabet_iterator=0;//iterator on the elements of the alphabet. used if alphabet is prepended to the text

	//input modes:
	FileReader fr;//from file
	string input="";//from string
	ulint global_position=0;//takes into account prefix/suffix added to the input stream
	ulint position=0;//current position of character to be read in the string/file stream (does not account for prefix/suffix)

	uint sigma=0;//alphabet size
	ulint number_of_phrases=0;
	string current_phrase=string();

	int last_perc=-1;//for verbose output

	options opt;

	bool file_closed=false;//file has been closed


};

typedef LZ77<bitv> lz77_t;
//typedef LZ77<bitvector_t<W_leafs,bv::alloc_immediatly> > lz77_t;

} /* namespace data_structures */
#endif /* LZ77_H_ */
