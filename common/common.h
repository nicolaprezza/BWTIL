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

/*
 * common.h
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
 *
 *  This header includes the others in common/, so it is sufficient to include only common.h in the other files
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <stdio.h>
#include <vector>
#include <cstring>
#include <cmath>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <sys/stat.h>
#include <set>
#include "stdint.h"
#include <sstream>
#include <algorithm>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fstream>
#include "bitvector.h"
#include "packed_view.h"
#include <assert.h>

using namespace std;
using namespace bv;

namespace bwtil{

#define VERBOSE_CHANNEL std::cout
#define ERROR_CHANNEL std::cerr

typedef uint64_t ulint;
typedef long int lint;
typedef unsigned int uint;
typedef unsigned short int t_char; ///< Type for char conversion
typedef unsigned short int t_errors; ///< Type for ERRORS

typedef unsigned char uchar;
typedef unsigned char symbol;
typedef unsigned char uint8;

typedef packed_view<vector> packed_view_t;
typedef bitview<vector> bitview_t;

constexpr uint W_leafs = 4096;//packed B-trees parameters
constexpr uint W_nodes = 256;

typedef bitvector_t<W_leafs> bitv;

enum input_mode {file_path,text};//input is a file path or a text string?

enum hash_type {DNA_SEARCH,BS_SEARCH,QUALITY_DNA_SEARCH,QUALITY_BS_SEARCH,DEFAULT};

#include "../extern/getRSS.h"

// Functions :

//Uncomment this if popcount is not available in hardware
/*inline int popcnt(ulint x){

	x = x - ((x&0xAAAAAAAAAAAAAAAA)>>1);//groups of 2 bits
	x = (x&0x3333333333333333)+((x&0xCCCCCCCCCCCCCCCC)>>2);//groups of 4 bits
	x = (x&0x0F0F0F0F0F0F0F0F)+((x&0xF0F0F0F0F0F0F0F0)>>4);//groups of 8 bits

	x = x+(x>>32);//accumulate 8 counters of 8 bits in 4 counters of 8 bits
	x = x+(x>>16);//accumulate 4 counters of 8 bits in 2 counters of 8 bits
	x = x+(x>>8);//accumulate 2 counters of 8 bits in 1 counter of 8 bits

	return x&0x00000000000000FF;

}*/

//Comment this if popcount is not available in hardware
#define popcnt(x) __builtin_popcountll(x)

/*
template <typename T>
inline void save_packed_view_to_file_T(packed_view_t pv, size_t size, FILE * fp){

	for(uint i=0;i<size;i++){
		T x = pv[i];
		fwrite(&x, sizeof(T), 1, fp);
	}

}

template <typename T>
inline packed_view_t load_packed_view_from_file_T(size_t width, size_t size, FILE * fp){

 packed_view_t pv = packed_view_t(width,size);

 ulint numBytes;
 for(uint i=0;i<size;i++){

	 T x;
	 numBytes=fread(&x, sizeof(T), 1, fp);
	 assert(numBytes>0);

	 pv[i] = x;

 }

 numBytes++;
 return pv;

}
*/

inline uint number_of_bits(ulint x){

	uint width = 0;
	while(x>0){
		width++;
		x = x>>1;
	}

	return width;
}

//number of bits required to write down x
uint8_t intlog2(ulint x){

	uint8_t res=0;

	while(x>0){
		x=x>>1;
		res++;
	}

	if(res==0) res = 1;

	return res;

}

inline void save_bitview_to_file(bitview_t bv, size_t size, FILE * fp){

	ulint i = 0;
	ulint x;

	while(i+64<=size){

		x = bv.get(i,i+64);
		fwrite(&x, sizeof(ulint), 1, fp);

		i+=64;

	}

	if(i<size){

		x = bv.get(i,size);
		fwrite(&x, sizeof(ulint), 1, fp);

	}

}

inline bitview_t load_bitview_from_file(bitview_t & bv, size_t size, FILE * fp){

	ulint i = 0;
	ulint x;

	ulint numBytes;

	while(i+64<=size){

		numBytes=fread(&x, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		bv.set(i,i+64,x);

		i+=64;

	}

	if(i<size){

		numBytes=fread(&x, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		bv.set(i,size,x);

	}

	 numBytes++;
	return bv;

}

inline bitview_t load_bitview_from_file(size_t size, FILE * fp){

	ulint i = 0;
	ulint x;

	bitview_t bv = bitview_t(size);
	ulint numBytes;

	while(i+64<=size){

		numBytes=fread(&x, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		bv.set(i,i+64,x);

		i+=64;

	}

	if(i<size){

		numBytes=fread(&x, sizeof(ulint), 1, fp);
		assert(numBytes>0);

		bv.set(i,size,x);

	}

	 numBytes++;
	return bv;

}

inline void save_packed_view_to_file(packed_view_t pv, size_t size,FILE * fp){

	save_bitview_to_file(pv.bits(), size*pv.width(), fp);

	/*if(pv.width()<=8)
		save_packed_view_to_file_T<uint8_t>(pv,size,fp);
	else if(pv.width()<=16)
		save_packed_view_to_file_T<uint16_t>(pv,size,fp);
	else if(pv.width()<=32)
		save_packed_view_to_file_T<uint32_t>(pv,size,fp);
	else
		save_packed_view_to_file_T<uint64_t>(pv,size,fp);*/

}

inline packed_view_t load_packed_view_from_file(size_t width, size_t size, FILE * fp){

	 packed_view_t pv = packed_view_t(width,size);
	 load_bitview_from_file(pv.bits(),size*width ,fp);

	 return pv;

	/*if(width<=8)
		return load_packed_view_from_file_T<uint8_t>(width, size, fp);
	if(width<=16)
		return load_packed_view_from_file_T<uint16_t>(width, size, fp);
	if(width<=32)
		return load_packed_view_from_file_T<uint32_t>(width, size, fp);

	return load_packed_view_from_file_T<uint64_t>(width, size, fp);*/

}

}
#endif /* COMMON_H_ */
