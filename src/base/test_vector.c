#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef int cache_line_vector __attribute__((vector_size(64)));

int kernel(cache_line_vector*  a, cache_line_vector  b){
    cache_line_vector tmp = *a;
//    tmp = tmp + b;
    return tmp[2];
}

int main(int argc, char** argv){
    cache_line_vector i = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    cache_line_vector j = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1,  1};
	int result = kernel(&i, j);
	printf("Result: %i\n", result);

	// Check if correct result
//	if (result == 3)
	if (result == 2)
		return 0;

	return 1;
}
