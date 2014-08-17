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
#include <sys/stat.h>
#include <set>
#include "stdint.h"
#include <sstream>
#include <algorithm>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fstream>
#include "../extern/bitvector/include/bitvector.h"
#include "../extern/bitvector/include/packed_view.h"

using namespace std;
using namespace bv;

#define VERBOSE_CHANNEL std::cout
#define ERROR_CHANNEL std::cerr

typedef uint64_t ulint;
typedef unsigned int uint;
typedef unsigned short int t_char; ///< Type for char conversion
typedef unsigned short int t_errors; ///< Type for ERRORS

typedef unsigned char uchar;
typedef unsigned char symbol;
typedef unsigned char uint8;

typedef packed_view<vector> packed_view_t;

constexpr uint W_leafs = 2048;//packed B-trees parameters
constexpr uint W_nodes = 256;

enum hash_type {DNA_SEARCH,BS_SEARCH,DEFAULT};

// Functions :

/*int popcnt(unsigned long int x){//no need for HW implementation

	x = x - ((x&0xAAAAAAAAAAAAAAAA)>>1);//groups of 2 bits
	x = (x&0x3333333333333333)+((x&0xCCCCCCCCCCCCCCCC)>>2);//groups of 4 bits
	x = (x&0x0F0F0F0F0F0F0F0F)+((x&0xF0F0F0F0F0F0F0F0)>>4);//groups of 8 bits

	x = x+(x>>32);//accumulate 8 counters of 8 bits in 4 counters of 8 bits
	x = x+(x>>16);//accumulate 4 counters of 8 bits in 2 counters of 8 bits
	x = x+(x>>8);//accumulate 2 counters of 8 bits in 1 counter of 8 bits

	return x&0x00000000000000FF;

}*/

//const-time popcnt (if available in hardware)
//#define popcnt(x) __builtin_popcount(x>>32)+__builtin_popcount(x&0x00000000FFFFFFFF);

#define popcnt(x) __builtin_popcountll(x)

#define check_numBytes() if (numBytes == 0) { VERBOSE_CHANNEL << "Read 0 bytes when reading dB-hash file (StaticBitVector error)" << endl << flush; exit(1); }

#include <memory>
#include <type_traits>
#include <utility>

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_helper(std::false_type, Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_helper(std::true_type, Args&&... args) {
   static_assert(std::extent<T>::value == 0,
       "make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

   typedef typename std::remove_extent<T>::type U;
   return std::unique_ptr<T>(new U[sizeof...(Args)]{std::forward<Args>(args)...});
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
   return make_unique_helper<T>(std::is_array<T>(), std::forward<Args>(args)...);
}

template <typename T>
void save_packed_view_to_file_T(packed_view_t pv, FILE * fp){

	for(uint i=0;i<pv.size();i++){
		T x = pv[i];
		fwrite(&x, sizeof(T), 1, fp);
	}

}

template <typename T>
packed_view_t load_packed_view_from_file_T(size_t width, size_t size, FILE * fp){

 auto pv = packed_view_t(width,size);

 ulint numBytes;
 for(uint i=0;i<pv.size();i++){

	 T x;
	 numBytes=fread(&x, sizeof(T), 1, fp);
	 check_numBytes();
	 pv[i] = x;

 }

 return pv;

}

void save_packed_view_to_file(packed_view_t pv, FILE * fp){

	if(pv.width()<=8)
		save_packed_view_to_file_T<uint8_t>(pv,fp);
	if(pv.width()<=16)
		save_packed_view_to_file_T<uint16_t>(pv,fp);
	if(pv.width()<=32)
		save_packed_view_to_file_T<uint32_t>(pv,fp);
	else
		save_packed_view_to_file_T<uint64_t>(pv,fp);

}

packed_view_t load_packed_view_from_file(size_t width, size_t size, FILE * fp){

	if(width<=8)
		return load_packed_view_from_file_T<uint8_t>(width, size, fp);
	if(width<=16)
		return load_packed_view_from_file_T<uint16_t>(width, size, fp);
	if(width<=32)
		return load_packed_view_from_file_T<uint32_t>(width, size, fp);

	return load_packed_view_from_file_T<uint64_t>(width, size, fp);

}






#endif /* COMMON_H_ */
