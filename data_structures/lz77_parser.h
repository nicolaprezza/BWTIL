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

/*
 * Builds online and in compressed space the LZ77 parse of an input stream.
 * Uses a dynamic compressed BWT as main structure. Zero-order compressed space, n log n time.
 *
 */
//============================================================================


#ifndef LZ77_PARSER_H_
#define LZ77_PARSER_H_

#include "../common/common.h"
#include "DynamicBWT.h"
#include "dynamic_vector.h"

namespace bwtil {

template <	typename bitvector_type = bitv,
			class dynamic_vector_type = dynamic_vector_t
		>
class lz77_parser {

public:

	typedef DynamicBWT<bitvector_type, dynamic_vector_type> dynamic_bwt_type;

	lz77_parser(){};

	/*
	 * token: an element of the parse
	 */
	struct token{

		string phrase;
		ulint start_position;
		//phrases composed only by the first occurrence of a character in the text do not have a defined start position
		bool start_position_is_defined;

	};

	/*
	 * Compute online the lz77 parse of the input stream. Requires as input also the characters with
	 * their absolute frequencies (in order to Huffman-compress the structures to H0).
	 * Note that the input stream must satisfy the declared alphabet/frequencies and file length
	 * (i.e. sum of all frequencies). Otherwise, the parser exits with an error.
	 *
	 * \param input: the input as an istream
	 * \param alphabet_and_frequencies: A set of pairs <char,freq>: the characters and their absolute
	 * 									frequency in the istream. A character must appear only once
	 * 									in this set. Call static function get_alphabet_and_frequencies
	 * 									to compute this parameter.
	 * \param sample_rate : Store SA pointers every sample_rate positions of the dynamic BWT
	 * \param verbose
	 *
	 *
	 */
	lz77_parser(std::istream & input, set<pair<uchar,ulint> > alphabet_and_frequencies, ulint sample_rate = 8, bool verbose = false){

		this->input = &input;
		this->verbose = verbose;

		//compute input length
		n = 0;
		for(auto s : alphabet_and_frequencies)
			n+=s.second;

		sigma = alphabet_and_frequencies.size();

		//mapping from uchar to int in {0,...,sigma-1}
		uchar_to_int = vector<uint>(256,sigma+1);

		//compute sigma and symbol remapping
		sigma = 0;
		for(auto s : alphabet_and_frequencies){

			assert(uchar_to_int[s.first]>sigma and "Error while lz77 parsing input stream : input alphabet has a repeated character");
			uchar_to_int[s.first] = sigma++;

		}

		//compute character frequencies
		vector<ulint> freqs(sigma,0);
		for(auto s : alphabet_and_frequencies)
			freqs[ uchar_to_int[s.first] ] = s.second;

		dbwt = dynamic_bwt_type(freqs, sample_rate);

		if(verbose)
			cout << "Parsing input (LZ77) ..." << endl;

	}

	token get_token(){

		assert(input!=0);
		assert(position<=n);

		if(verbose){

			int perc = ((position*100)/n);
			if(perc>=last_perc+5){

				cout << perc << "% done ..." << endl;
				last_perc = perc;

			}

		}

		//if eof reached, return empty token
		if(position==n)
			return {string(),0,false};

		//current interval in the BWT. Start with full BWT
		pair<ulint,ulint> interval = {0,dbwt.size()};

		//position on the BWT of previous occurrence of a factor.
		//if position_on_bwt == dbwt.getMaxLength(), position is
		//not defined (first occurrence of a character)
		ulint position_on_bwt = dbwt.getMaxLength();

		//read character from the stream, extend the BWT interval
		char c = read_char();
		interval = dbwt.BS(interval,uchar_to_int[(uchar)c]);

		//current phrase: at least 1 character
		string phrase;

		//if interval is empty here, we have the first occurrence of a character
		if(interval.second <= interval.first){

			//extend BWT with the character
			dbwt.extend(uchar_to_int[(uchar)c]);

			//we haven't already read following character
			mismatching_character=0;

			//phrase is only c
			phrase = string()+c;
			return {phrase,0,false};

		}

		//cout << c << " " << interval.first << " " << interval.second << endl;

		position_on_bwt = interval.first;

		if(position==n){

			//phrase is only c
			phrase = string()+c;

			//character occurs before, so locate an occurrence
			//dbwt.locate_right(position_on_bwt) returns position of the occurrence
			//counting positions from the right. This is what we want since
			//we are building the BWT of the reversed text. We subtract 1
			//because there is a terminator character.
			return {phrase, dbwt.locate_right(position_on_bwt)-1, true};

		}

		//extend BWT until possible
		while(interval.second > interval.first and position < n){

			//append c to the current phrase
			phrase = string(phrase)+c;

			//extend BWT and obtain position where new suffix has been inserted
			ulint o = dbwt.extend(uchar_to_int[(uchar)c]);

			//careful here! if we inserted the new suffix in position interval.first,
			//we do not want to return position interval.first as occurrence of this
			//phrase, since interval.first is the position of the phrase itself!
			//instead, return interval.first+1 (safe since after the insertion there are
			//at least 2 occurrences of the phrase), which is the position
			//of a previous occurrence.
			if(o==interval.first)
				position_on_bwt = interval.first+1;
			else
				position_on_bwt = interval.first;

			//we just inserted a new suffix in the current interval:
			//extend right side of the interval.
			interval.second++;

			//search next character
			c = read_char();

			interval = dbwt.BS(interval,uchar_to_int[(uchar)c]);

			//searching this character caused the interval to become empty:
			//undo the read (i.e. next call to read_char() will return c)
			if(interval.second <= interval.first){

				mismatching_character = c;
				position--;

			}else if(position==n){

				//I found the character, but stream is finished:
				//extend phrase and bwt

				phrase = string(phrase)+c;

				ulint o = dbwt.extend(uchar_to_int[(uchar)c]);

				if(o==interval.first)
					position_on_bwt = interval.first+1;
				else
					position_on_bwt = interval.first;

				interval.second++;

			}

		}

		return {phrase,dbwt.locate_right(position_on_bwt)-phrase.size(),true};

	}

	/*
	 * returns total size of the stream
	 */
	ulint size(){

		assert(input!=0);
		return n;

	}

	/*
	 * true if end of input stream has been reached
	 */
	bool eof(){

		assert(input!=0);
		assert(position<=n);

		return position >= n;

	}

	/* Compute alphabet and frequencies of an input stream. To be used wile calling
	 * the lz77_parser constructor.
	 *
	 * \param is  the input stream
	 * \returns a set of pairs <c, freq> for each character c in the stream. freq is the
	 * 			absolute frequency of c in the stream.
	 *
	 */
	static set<pair<uchar,ulint> > get_alphabet_and_frequencies(std::istream &is){

		set<pair<uchar,ulint> > result;

		//lambda comparator for elements in the set: look only at the
		//first element of the pair
		auto comp = [](pair<char,ulint> a, pair<char,ulint> b){
			return a.first < b.first;
		};

		set<pair<char,ulint>, decltype(comp) > ch_and_freq(comp);

		//read the stream and compute alphabet and frequencies
		char c;
		is.read(&c,1);

		while(not is.eof()){

			auto it = ch_and_freq.find({c,0});

			if( it == ch_and_freq.end() ){

				//if the character is not present in the set, then insert it with counter 1
				ch_and_freq.insert({c,1});

			}else{

				//else: increment counter associated with the character
				ulint count = it->second+1;
				ch_and_freq.erase(it);
				ch_and_freq.insert({c,count});

			}

			is.read(&c,1);

		}

		for(auto p : ch_and_freq)
			result.insert({(uchar)p.first, p.second});

		return result;

	}

private:

	char read_char(){

		char c=mismatching_character;

		if(c==0)
			input->read(&c,1);
		else
			mismatching_character = 0;

		position++;

		return c;

	}

	//the dynamic compressed BWT
	dynamic_bwt_type dbwt;

	//the input stream
	std::istream * input = 0;

	//remapping of symbols:
	vector<uint> uchar_to_int;

	ulint position = 0;//current position of character to be read in the istream

	uint sigma = 0;//alphabet size

	ulint n = 0;//stream length

	char mismatching_character=0;//first character after previous read phrase

	bool verbose=false;

	int last_perc=0;//for verbose output

};

} /* namespace data_structures */


#endif /* LZ77_PARSER_H_ */
