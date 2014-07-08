/*
 * CumulativeCounters.cpp
 *
 *  Created on: Jun 22, 2014
 *      Author: nicola
 */

#include "CumulativeCounters.h"

namespace bwtil {

CumulativeCounters::CumulativeCounters(uint sigma, ulint n){//size of the alphabet and maximum number to be stored in a counter

	if(n==0){//empty counters
		empty=true;
		return;
	}

	base_counter = 0;

	empty=false;
	this->sigma=sigma;

	log2n = ceil(log2(n+1));

	uint w=64;
	d = w/log2n;

	ones = (ulint)0;
	for(uint i=0;i<d;i++)
		ones = ones | (((ulint)1)<<(i*log2n));

	nr_of_leafs = sigma/d + (sigma%d==0?0:1);//leafs

	uint nr_of_nodes_in_level = nr_of_leafs;
	nr_of_nodes = nr_of_nodes_in_level;

	//round nr_of_nodes_in_level to the next power of d+1

	uint nodes_pow = 1;
	while(nodes_pow<nr_of_nodes_in_level)
		nodes_pow *= (d+1);

	nr_of_nodes_in_level = nodes_pow;
	uint height=1;

	while(nr_of_nodes_in_level>1){

		height++;
		nr_of_nodes_in_level = nr_of_nodes_in_level/(d+1);
		nr_of_nodes += nr_of_nodes_in_level;

	}

	/*cout<<"log2n="<<(uint)log2n<<endl;
	cout << "d="<<(uint)d<<endl;
	cout << "nr_of_leafs="<<(uint)nr_of_leafs<<endl;
	cout << "height="<<height<<endl;
	cout << "nr_of_nodes="<<(uint)nr_of_nodes<<endl;*/

	nodes = new ulint[nr_of_nodes];
	for(uint i=0;i<nr_of_nodes;i++)//reset all counters
		nodes[i]=0;

}

string CumulativeCounters::toString(){

#ifdef DEBUG
	if(empty){
		cout << "ERROR (CumulativeCounters): toString() called on empty counter\n";
		exit(0);
	}
#endif

	stringstream ss;

	 for(uint i =0;i<sigma;i++)
		ss << getCount(i) << " ";

	 return ss.str();

}

void CumulativeCounters::incrementFrom(ulint * node, uint i){

#ifdef DEBUG
	if(empty){
		cout << "ERROR (CumulativeCounters): incrementFrom() called on empty counter\n";
		exit(0);
	}
#endif

#ifdef DEBUG
	if(i>d){
		cout << "ERROR (CumulativeCounters): incrementing counter i>d (" << i << ">" << d << ")\n";
		exit(0);
	}
#endif

	ulint MASK = ~((ulint)0);

	if(i>0) MASK = ((ulint)1<<((d-i)*log2n))-1;

	//printWord(MASK&ones);

	*node = *node + (MASK&ones);

}

ulint CumulativeCounters::getCounterNumber(ulint n, uint i){//get value of the counter number 0<=i<d in the node n

#ifdef DEBUG
	if(empty){
		cout << "ERROR (CumulativeCounters): getCounterNumber() called on empty counter\n";
		exit(0);
	}
#endif

#ifdef DEBUG
	if(i>=d){
		cout << "ERROR (CumulativeCounters): get counter i>=d (" << i << ">=" << d << ")\n";
		exit(0);
	}
#endif

	ulint MASK = (((ulint)1)<<log2n)-1;

	return (n>>((d-i-1)*log2n))&MASK;

}

uint16_t CumulativeCounters::child(uint16_t n, uint8_t i){//return child number 0<=i<=d of node n

#ifdef DEBUG
	if(empty){
		cout << "ERROR (CumulativeCounters): child() called on empty counter\n";
		exit(0);
	}
#endif

#ifdef DEBUG
	if(i>d){
		cout << "ERROR (CumulativeCounters): child number i>d (" << i << ">" << d << ")\n";
		exit(0);
	}
#endif

	return (n*(d+1))+i+1;

}


uint16_t CumulativeCounters::parent(uint16_t n){//return parent of node n

#ifdef DEBUG
	if(empty){
		cout << "ERROR (CumulativeCounters): parent() called on empty counter\n";
		exit(0);
	}
#endif

#ifdef DEBUG
	if(n>=nr_of_nodes){
		cout << "ERROR (CumulativeCounters): parent of inexistent node " << n <<"\n";
		exit(0);
	}
#endif

	return (n-(((n-1)%(d+1))+1))/(d+1);

}

uint8_t CumulativeCounters::childNumber(uint16_t n){//return which children number is n in his parent

#ifdef DEBUG
	if(n>=nr_of_nodes){
		cout << "ERROR (CumulativeCounters): parent of inexistent node " << n << "\n";
		exit(0);
	}
#endif

	return (n-1)%(d+1);

}

void CumulativeCounters::increment(symbol s){

#ifdef DEBUG
	if(empty){
		cout << "ERROR (CumulativeCounters): increment() called on empty counter\n";
		exit(0);
	}
#endif

#ifdef DEBUG
	if(s>=sigma){
		cout << "ERROR (CumulativeCounters): symbol " << s << " not in alphabet.\n";
		exit(0);
	}
#endif

	uint current_node = (nr_of_nodes - nr_of_leafs) + (s/d);//offset leafs + leaf number
	uint offset_in_node = s%d;//number of the counter inside the node

	incrementFrom(&nodes[current_node], offset_in_node);//increment counters in the leaf

	while(current_node>0){//repeat while current node is not the root

		offset_in_node = childNumber(current_node);
		current_node = parent(current_node);

		incrementFrom(nodes+current_node, offset_in_node);

	}

}

ulint CumulativeCounters::getCount(symbol s){

#ifdef DEBUG
	if(empty){
		cout << "ERROR (CumulativeCounters): getCount() called on empty counter\n";
		exit(0);
	}
#endif

	if(s==0)
		return base_counter;

	s--;

#ifdef DEBUG
	if(s>=sigma){
		cout << "ERROR (CumulativeCounters): symbol " << s << " not in alphabet.\n";
		exit(0);
	}
#endif

	uint current_node = (nr_of_nodes - nr_of_leafs) + (s/d);//offset leafs + leaf number

	uint offset_in_node = s%d;//number of the counter inside the node

	ulint count = getCounterNumber(nodes[current_node],offset_in_node);

	while(current_node>0){//repeat while current node is not the root

		offset_in_node = childNumber(current_node);
		current_node = parent(current_node);

		if(offset_in_node>0)
			count += getCounterNumber(nodes[current_node],offset_in_node-1);

	}

	return count + base_counter;

}


} /* namespace compressed_bwt_construction */
