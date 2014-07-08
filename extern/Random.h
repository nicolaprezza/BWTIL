/*
 * Random.h
 *
 *  Created on: 16/ott/2013
 *      Author: cdf
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include <boost/random.hpp>

#include <climits>

namespace bwtil {

typedef boost::mt19937 random_t;
typedef boost::uniform_real<> range_t;
typedef boost::variate_generator<random_t&, range_t > generator_t;

class Random {
public:
	static Random* getInstance();
	void finalize();

	inline double nextDouble() {
		return (*generator)();
	}

	inline unsigned int nextUInt(unsigned int max) {
		return (unsigned int) ((*generator)() * max);
	}

	inline int nextInt(int max) {
		return (int) ((*generator)() * max);
	}

	inline int nextInt(int min, int max) {
		return (int) ((*generator)() * (max-min)) + min;
	}

protected:
	virtual ~Random();
	Random();
	static Random* _instance;

	random_t * random;
	generator_t * generator;

};

} /* namespace data_structures */
#endif /* RANDOM_H_ */
