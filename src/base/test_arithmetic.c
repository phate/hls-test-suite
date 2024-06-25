#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int kernel(int a, int b){
	int c = ((a + b) * b - a) / b % a;
	return c;
}

int main(int argc, char** argv){
	int result = kernel(4, 8);
	printf("Result: %i\n", result);

	// Check if correct result
	if (result == 3)
		return 0;

	return 1;
}
