#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int kernel(int array[5]){
	array[2] = 6;
	return array[2];
}

int main(int argc, char** argv){
	int array[5] = {0, 1, 2, 3, 4};
	int result = kernel(array);
	printf("Result: %i\n", result);

	// Check if correct result
	if (result == 6)
		return 0;

	return 1;
}
