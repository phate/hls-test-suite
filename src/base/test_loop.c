#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int kernel(int a, int b) {
	for (int i=0; i<b; i++) {
		a++;
	}
	return a;
}

int main(int argc, char** argv) {
	int result = kernel(1, 5);
	printf("Result: %i\n", result);

	// Check if correct result
	if (result == 6)
		return 0;

	return 1;
}
