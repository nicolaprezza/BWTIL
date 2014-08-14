//============================================================================
// Name        : HashFunction.cpp
// Author      : Nicola Prezza and Alberto Policriti
// Version     : 1.0
// Copyright   : GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
// Description : de Bruijn hash function to be used in the dB-hash.
//============================================================================

#include "HashFunction.h"

namespace bwtil {

/*
 * WARNING: use DNA_SEARCH or BS_SEARCH as hash type only if the text is on the alphabet {A,C,G,T,N}
 */
HashFunction::HashFunction(ulint n, ulint m, hash_type type, bool verbose){

	if(verbose)
		cout << "\nBuilding hash function ... \n";

	this->type = type;
	this->m=m;

	code = new uint[256];

	for(uint i=0;i<256;i++)
		code[i]=0;

	random_char = new bool[256];

	if(type == DEFAULT)
		init_DEFAULT();

	if(type == DNA_SEARCH)
		init_DNA_SEARCH();

	if(type == BS_SEARCH)
		init_BS_SEARCH();

	w = ceil(log2(m*n)/log2(base));//optimal word size
	log_base = ceil(log2(base));
	r = m%w;

	if(verbose) cout << " File length : n = " << n<<endl;

	if(verbose)	cout << " Pattern length : m = " << m<<endl;

	if(verbose)	cout << " Alphabet size : sigma = " << base<<endl;

	if(verbose)	cout << " Fingerprints base : b = " << (1<<log_base)<< " (" << log_base << " bits per digit)" << endl;

	if(verbose)	cout << " Optimal word length : w = " << w<<endl << " Building Z set ... ";

	buildZSet();

	if(verbose)	cout << "Done." <<endl;

	if(verbose)	cout << "Done." <<endl;

}

HashFunction::HashFunction(){}

HashFunction::HashFunction(ulint m, const char * file_path, bool verbose){

	if(verbose)	cout << "\nBuilding hash function ... \n";

	this->type = DEFAULT;
	this->m=m;

	code = new uint[256];

	random_char = new bool[256];
	for(uint i=0;i<256;i++)
		random_char[i]=false;

	ulint n=0;

	if(verbose)	cout << " Reading input file for detecting alphabet size and file length ... \n";

	unsigned char ch;
	fstream fin(file_path, fstream::in);

	vector<uchar> alphabet;
	vector<bool> char_inserted = vector<bool>(256,false);

	while (fin >> noskipws >> ch) {

		if(not char_inserted.at(ch)){

			alphabet.push_back(ch);
			char_inserted.at(ch)=true;

		}

		n++;

	}

	if(n==0){
		VERBOSE_CHANNEL<< "Empty file. "  << file_path <<endl;
		fin.close();
		exit(1);
	}

	if(alphabet.size()<=1){
		VERBOSE_CHANNEL<< "Error: alphabet size (" << alphabet.size() << ") is less than or equal to 1." <<endl;
		fin.close();
		exit(1);
	}

	log_base = ceil(log2(alphabet.size()));
	base = ((ulint)1)<<log_base;

	//sort alphabet

	std::sort(alphabet.begin(),alphabet.end());

	//calculate code

	for(uint i=0;i<256;i++)
		code[i]=0;

	for(uint i=0;i<alphabet.size();i++)
		code[alphabet.at(i)] = i;

	fin.close();

	if(verbose)	cout << " File length : n = " << n<<endl;

	if(verbose)	cout << " Pattern length : m = " << m<<endl;

	w = ceil(log2(m*n)/log_base);//optimal word size

	if(w>m) w=m;

	r = m%w;

	if(verbose)	cout << " Alphabet size : sigma = " << alphabet.size() << endl;

	if(verbose)	cout << " Fingerprints base : b = " << base << " (" << log_base << " bits per digit)" << endl;

	if(verbose) cout << " Optimal word length : w = " << w<<endl << " Building Z set ... ";

	buildZSet();

	if(verbose)	cout << "Done." <<endl;

	if(verbose)	cout << "Done." <<endl;

}

void HashFunction::init_DEFAULT(){

	//fingerprints are base 4. A character c is assigned the digit c%4.

	base = 4;

	for(uint i=0;i<256;i++){
		code[i]=i%base;
		random_char[i] = false;
	}

}

void HashFunction::init_DNA_SEARCH(){

	for(uint i=0;i<256;i++)
		random_char[i] = true;

	random_char[(uint) 'a'] = random_char[(uint) 'A'] = false;
	random_char[(uint) 'c'] = random_char[(uint) 'C'] = false;
	random_char[(uint) 'g'] = random_char[(uint) 'G'] = false;
	random_char[(uint) 't'] = random_char[(uint) 'T'] = false;

	base = 4;

	code[(uint) 'a'] = code[(uint) 'A'] = 0;
	code[(uint) 'c'] = code[(uint) 'C'] = 1;
	code[(uint) 'g'] = code[(uint) 'G'] = 2;
	code[(uint) 't'] = code[(uint) 'T'] = 3;
	code[(uint) 'n'] = code[(uint) 'N'] = 0;
	code[(uint) '$'] = 1;

}

void HashFunction::init_BS_SEARCH(){

	for(uint i=0;i<256;i++)
		random_char[i] = true;

	random_char[(uint) 'a'] = random_char[(uint) 'A'] = false;
	random_char[(uint) 'c'] = random_char[(uint) 'C'] = false;
	random_char[(uint) 'g'] = random_char[(uint) 'G'] = false;
	random_char[(uint) 't'] = random_char[(uint) 'T'] = false;

	base=2;

	code[(uint) 'a'] = code[(uint) 'A'] = 0;
	code[(uint) 'c'] = code[(uint) 'C'] = 1;
	code[(uint) 'g'] = code[(uint) 'G'] = 0;
	code[(uint) 't'] = code[(uint) 'T'] = 1;
	code[(uint) 'n'] = code[(uint) 'N'] = 0;
	code[(uint) '$'] = 1;

}

ulint HashFunction::hashValue(string P){//compute fingerprint of pattern P of length m.

	ulint W = 0;
	ulint result = 0;

	for(uint i=0;i<m-r;i++){

		if(i%w==0){
			result = result^W;
			W = 0;
		}

		W = (W<<log_base) + code[(unsigned int)P.at(i)];

	}

	result = result^W;
	W = 0;

	if(r>0){

		for(uint i=m-w;i<m;i++)
			W = (W<<log_base) + code[(unsigned int)P.at(i)];

		result = result^W;

	}

	return result;

}

ulint HashFunction::hashValue(unsigned char *P){//compute fingerprint of pattern P of length m.

	ulint W = 0;
	ulint result = 0;

	for(uint i=0;i<m-r;i++){

		if(i%w==0){
			result = result^W;
			W = 0;
		}

		W = (W<<log_base) + code[P[i]];

	}

	result = result^W;
	W = 0;

	if(r>0){

		for(uint i=m-w;i<m;i++)
			W = (W<<log_base) + code[P[i]];

		result = result^W;

	}

	return result;

}//hash value

unsigned char* HashFunction::hashValue(unsigned char *P, ulint n){

	ulint length=n-m+w;
	int blocks = m/w;

	ulint bufsize=m+1;

	int r = m%w;

	if(r==0){
		blocks--;
		r=w;
	}

	//there are 'blocks' blocks of length w and a final block of length r (which can be equal to w)

	srand(time(NULL));

	unsigned char *res = new unsigned char[length];//fingerprint of P

	if(m==w){//identity function

		for(ulint i=0;i<length;i++)
			if(random_char[P[i]])
				res[i] = rand()%(base);
			else
				res[i] = code[P[i]];

		return res;

	}

	vector<unsigned char> buffer(bufsize);

	//fill buffer with the first m+1 digits
	for(ulint i=0;i<bufsize;i++){
		if(random_char[P[i]])
			buffer[i] = rand()%(base);
		else
			buffer[i] = code[P[i]];
	}

	//compute first w digits
	for(ulint i=0;i<w;i++){

		res[i]=0;
		for(int j=0;j<blocks;j++)
			res[i] ^= buffer[(i+j*w)%bufsize];

		res[i] ^= buffer[(i+(blocks-1)*w+r)%bufsize];

	}

	//compute the other digits
	ulint newpos=0;

	for(ulint i=w;i<length;i++){

		res[i] = 	res[i-w] ^
					buffer[(i-w)%bufsize] ^
					buffer[((int)i+(blocks-2)*(int)w+r)%bufsize] ^
					buffer[(i+(blocks-1)*w)%bufsize] ^
					buffer[(i+(blocks-1)*w + r)%bufsize];

		newpos=i+m-w+1;

		if(newpos<n){

			if(random_char[P[newpos]])
				buffer[newpos%bufsize] = rand()%(base);
			else
				buffer[newpos%bufsize] = code[P[newpos]];

		}

	}

	return res;

}//hashValue

unsigned char* HashFunction::hashValueRemapped(unsigned char *P, ulint n){

	ulint length=n-m+w;
	int blocks = m/w;

	ulint bufsize=m+1;

	int r = m%w;

	if(r==0){
		blocks--;
		r=w;
	}

	srand(time(NULL));

	//there are 'blocks' blocks of length w and a final block of length r (which can be equal to w)

	unsigned char *res = new unsigned char[length+1];//fingerprint of P plus 0x0 byte appended

	if(m==w){//identity function

		for(ulint i=0;i<length;i++)
			if(random_char[P[i]])
				res[i] = rand()%(base)+1;
			else
				res[i] = code[P[i]]+1;

		res[length] = 0;

		return res;

	}

	vector<unsigned char> buffer(bufsize);

	//fill buffer with the first m+1 digits
	for(ulint i=0;i<bufsize;i++){
		if(random_char[P[i]])
			buffer[i] = rand()%(base);
		else
			buffer[i] = code[P[i]];
	}

	//compute first w digits
	for(ulint i=0;i<w;i++){

		res[i]=0;
		for(int j=0;j<blocks;j++)
			res[i] ^= buffer[(i+j*w)%bufsize];

		res[i] ^= buffer[(i+(blocks-1)*w+r)%bufsize];

	}

	//compute the other digits
	ulint newpos=0;

	for(ulint i=w;i<length;i++){

		res[i] = 	res[i-w] ^
					buffer[(i-w)%bufsize] ^
					buffer[((int)i+(blocks-2)*(int)w+r)%bufsize] ^
					buffer[(i+(blocks-1)*w)%bufsize] ^
					buffer[(i+(blocks-1)*w + r)%bufsize];

		newpos=i+m-w+1;

		if(newpos<n){

			if(random_char[P[newpos]])
				buffer[newpos%bufsize] = rand()%(base);
			else
				buffer[newpos%bufsize] = code[P[newpos]];

		}

	}

	for(ulint i=0;i<length;i++)//re-map adding 1 to each digit
		res[i]++;

	res[length] = 0;//append 0x0 byte

	return res;

}//hashValueRemapped

uint HashFunction::digitAt(ulint W, uint i){//i-th digit from right

	uint mask = (1<<log_base)-1;

	return (W>>(log_base*i)) & mask;

}

string HashFunction::toString(ulint W){//converts fingerprint of length w to string

	ostringstream ss;

	for(uint i=0;i<w;i++)
		ss << digitAt(W,w-i-1);

	return ss.str();

}

void HashFunction::buildZSet(){

	/*
	 * Z(0) = {0}
	 *
	 * Z(1) contains: (all numbers are in base 'base')
	 *
	 * 	if m mod w > 0
	 *
	 * 		- all words of the form 0^i x 0^(r-1) x 0^(k-r-1-i), i=0,...,w-r-1, x in {1,...,base-1}  where r=m mod w
	 *  	- if m>=2w			all words of the form 0^i x 0^(w-i-1), i=0,...,k-1, x in {1,...,base-1}
	 *  	- if m < 2w		all words of the form 0^i x 0^(w-i-1), i=0,...,m-w-1 or i = 2w-m,...,w-1, x in {1,...,base-1}
	 *
	 *  if m mod w = 0
	 *
	 *  	- all words of the form 0^i x 0^(k-i-1), i=0,...,w-1, x in {1,...,base-1}
	 *
	 *	Z(j), j>1 contains all the xor combinations of Z(j-1) and Z(1) elements
	 *
	 *
	 */

	Z_size = new uint[radius+1];
	Z = new ulint*[radius+1];

	Z_size[0] = 1;

	Z[0] = new ulint[1];
	Z[0][0] = 0;

	uint idx = 0;

	// build Z[1]

	if (r==0) {//m mod w == 0

		Z_size[1] = w*(base-1);

		Z[1] = new ulint[ Z_size[1] ];

		idx=0;

		for (ulint x=1;x<base;x++)//type of error
			for (uint i=0;i<w;i++){//position from right
				Z[1][idx] = (x << (log_base*i));
				idx++;
			}


	}else{

		if (m>=2*w or 2*r>=w){

			Z_size[1] = w*(base-1) + (w-r)*(base-1);
			Z[1] = new ulint[ Z_size[1] ];

			idx = 0;

			//1) fingerprints with only 1 nonzero digit

			for (ulint x=1;x<base;x++)//type of error
				for (uint i=0;i<w;i++){
					Z[1][idx] = (x << (log_base*i));
					idx++;
				}

			//1) fingerprints with 2 nonzero digits separated by r-1 zeroes

			ulint mask = 1 + (1<<(log_base*r));

			for (ulint x=1;x<base;x++)//type of error
				for (uint i=0;i<w-r;i++){//position

					Z[1][idx] = (mask*x)<<(log_base*i);
					idx++;

				}

		} else {

			Z_size[1] = (2*m - 2*w)*(base-1) + (w-r)*(base-1);
			Z[1] = new ulint[ Z_size[1] ];

			idx = 0;

			for (ulint x=1;x<base;x++)
				for (uint i=0;i<m-w;i++){

					Z[1][idx] = (x << (log_base*i));
					idx++;

				}

			for (ulint x=1;x<base;x++)
				for (uint i=2*w-m;i<w;i++){

					Z[1][idx] = (x << (log_base*i));
					idx++;

				}

			ulint mask = 1 + (1<<(log_base*r));

			for (ulint x=1;x<base;x++)
				for (uint i=0;i<w-r;i++){

					Z[1][idx] = (mask*x)<<(log_base*i);
					idx++;

				}
		}
	}

	quickSort(Z[1],Z_size[1]);

	ulint size;
	ulint *AllCombinations;

	for(uint err=2;err<=radius;err++){//build Z[2,...,radius]

		//Z[i] is the xor combination of all elements in Z[1] and in Z[i-1]
		size = Z_size[1]*Z_size[err-1];

		AllCombinations = new ulint[size];

		idx=0;
		for(uint i=0;i<Z_size[1];i++)
			for(uint j=0;j<Z_size[err-1];j++){

				AllCombinations[idx] = Z[1][i] ^ Z[err-1][j];//XOR between one element of Z[1] and one of Z[err-1]
				idx++;

			}

		//sort
		quickSort(AllCombinations,size);

		//now remove duplicates
		Z_size[err]=0;

		for(uint i=0;i<size;i++)
			if( (not searchInZ(AllCombinations[i],err) ) and (i<1 || AllCombinations[i]!=AllCombinations[i-1])  )
				Z_size[err]++;

		Z[err] = new ulint[ Z_size[err] ];

		idx=0;
		for(uint i=0;i<size;i++)
			if( (not searchInZ(AllCombinations[i],err) ) and (i<1 || AllCombinations[i]!=AllCombinations[i-1])  ){
				Z[err][idx] = AllCombinations[i];
				idx++;
			}

		delete [] AllCombinations;

		//cout << "Z(" << err << ") contains " << nr_of_elements[err] << " elements. " << endl;

	}//build Z[2,...,maxErr]


}

void HashFunction::quickSort(ulint *arr, uint n) {

	if(n<2) return;

	ulint pivot = arr[rand()%(n)];
	ulint t;
	uint i=0,j=0;

	while(i<n && arr[i]<pivot)
		i++;

	j=i;

	//invariant: arr[0,...,i-1]<pivot; arr[i,...,j]>=pivot
	while (j<n) {

		while(j<n && (arr[j]>=pivot))
			j++;

		if(j<n){//swap arr[i] and arr[j]
			t = arr[i];
			arr[i] = arr[j];
			arr[j] = t;
			i++;
		}

	}

	if(i==0){//pivot is the minimum element: avoid loop

		j=0;
		while(arr[j]!=pivot)
			j++;

		t = arr[i];
		arr[i] = arr[j];
		arr[j] = t;

		i++;
	}

	quickSort(arr,i);
	quickSort(arr+i,n-i);

}

bool HashFunction::binarySearch(ulint *arr, uint n, ulint x){

	int a=0,b=n-1;
	uint c;

	bool found=false;
	while((not found) and b>a){

		c = (b-a)/2 + a;

		if (x>arr[c]) {

			a = c+1;

		}else if (x<arr[c]) {

			b = c-1;

		}else
			found=true;

	}

	return found;

}//binary search

//search x in Z[1,...,err-1]
bool HashFunction::searchInZ(ulint x, uint err){

	bool found = false;

	for(uint i=0;i<err;i++)
		found = found or binarySearch(Z[i],Z_size[i],x);

	return found;

}

/**
 * nextElement() : return element and move the pointer to the next Z element
 */
ulint HashFunction::SetZIterator::nextElement() {

	ulint zel = 0;

	if(hasNext())
		zel = Z[current_errors][index];
	else
		return zel;

	index++;

	if(index>=Z_size[current_errors]){

		index = 0;
		current_errors++;

	}

	return zel;

}

void HashFunction::saveToFile(FILE *fp){

	fwrite(&base, sizeof(uint), 1, fp);
	fwrite(&m, sizeof(uint), 1, fp);
	fwrite(&w, sizeof(uint), 1, fp);
	fwrite(&r, sizeof(uint), 1, fp);
	fwrite(&log_base, sizeof(uint), 1, fp);
	fwrite(&type, sizeof(hash_type), 1, fp);

	fwrite(code, sizeof(uint), 256, fp);
	fwrite(random_char, sizeof(bool), 256, fp);

}

void HashFunction::loadFromFile(FILE *fp){

	ulint numBytes;

	numBytes = fread(&base, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&m, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&w, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&r, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&log_base, sizeof(uint), 1, fp);
	check_numBytes();
	numBytes = fread(&type, sizeof(hash_type), 1, fp);
	check_numBytes();

	code = new uint[256];
	random_char = new bool[256];

	numBytes = fread(code, sizeof(uint), 256, fp);
	check_numBytes();
	numBytes = fread(random_char, sizeof(bool), 256, fp);
	check_numBytes();

	buildZSet();

}




}
