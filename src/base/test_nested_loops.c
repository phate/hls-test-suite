#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int kernel(int a, int b) {
	a = 1;
	for (int i=0; i<5; i++) {
		for (int j=0; j<5; j++) {
			a++;
		}
	}
	return a;
}

int main(int argc, char** argv) {
	int result = kernel(1, 2);
	printf("Result: %i\n", result);

	// Check if correct result
	if (result == 26)
		return 0;

	return 1;
}
