#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int kernel(float a, int bi){
    float b = bi;
	int c = (b - a) * b;
	return c;
}

int main(int argc, char** argv){
	int result = kernel(4, 8);
	printf("Result: %d\n", result);

	// Check if correct result
	if (result == 32)
		return 0;

	return 1;
}
