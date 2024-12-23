#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int kernel(int a, int b) {
	for (int i=0; i<b; i++) {
		for (int j=0; j<b; j++) {
			a++;
		}
	}
	return a;
}

int main(int argc, char** argv) {
	int result = kernel(1, 5);
	printf("Result: %i\n", result);

	// Check if correct result
	if (result == 26)
		return 0;

	return 1;
}
