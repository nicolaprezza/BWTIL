/*
 * common.h
 *
 *  Created on: Jun 15, 2014
 *      Author: nicola
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

enum hash_type {DNA_SEARCH,BS_SEARCH,DEFAULT};

#endif /* COMMON_H_ */
