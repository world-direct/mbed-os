#pragma once
#include <stdlib.h>

class Median {

public:
	static int compute(int * buffer, int size);
	
private:
	Median(){};
	static int compare(const void * a, const void * b) { return ( *(int*)a - *(int*)b ); };
	
};