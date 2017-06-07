#include "Median.h"

int Median::compute(int * buffer, int size) {
	
	qsort(buffer, size, sizeof(int), compare);
	
	if(size % 2 == 0)
	{
		return (buffer[(size / 2) - 1] + buffer[size / 2]) / 2;
	}
	else
	{
		return buffer[(size - 1) / 2];
	}
	
}
