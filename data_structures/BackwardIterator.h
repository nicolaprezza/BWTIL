/*
 * BackwardIterator.h
 *
 *  Created on: Aug 16, 2014
 *      Author: nicola
 */

#ifndef BACKWARDITERATOR_H_
#define BACKWARDITERATOR_H_

namespace bwtil {

class BackwardIterator{

public:

	BackwardIterator(){};

	virtual ~BackwardIterator(){};

	virtual void rewind(){};

	virtual symbol read(){return 0;};

	virtual bool begin(){return true;};

	virtual void close(){};

	virtual ulint length(){return 0;};

};

}

#endif /* BACKWARDITERATOR_H_ */
