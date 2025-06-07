#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

float kernel(float a, float b){
	return a + b;
}

int main(int argc, char** argv){
	float result = kernel(8, -4);
	printf("Result: %f\n", result);

	// Check if correct result
	if (result == 4)
		return 0;

	return 1;
}
