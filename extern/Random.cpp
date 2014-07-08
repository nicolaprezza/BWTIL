/*
 * Random.cpp
 *
 *  Created on: 16/ott/2013
 *      Author: cdf
 */

#include "Random.h"

namespace bwtil {

Random * Random::_instance = NULL;


Random::~Random() {
	delete generator;
	delete random;
}

Random::Random() {
	random = new random_t;
	generator = new generator_t(*random, range_t(0,1));
	random->seed(time(0));
}

/* static */
Random* Random::getInstance() {
	if (_instance == NULL) {
		_instance = new Random;
	}
	return _instance;
}

void Random::finalize() {
	if (_instance != NULL) {
		delete _instance;
		_instance = NULL;
	}
}


} /* namespace data_structures */
