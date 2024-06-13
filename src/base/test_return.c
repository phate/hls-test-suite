#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int run(){
	return 5;
}

int main(int argc, char** argv){
	int result = run();
	printf("Result: %i\n", result);

	// Check if correct result
	if (result == 5)
		return 0;

	return 1;
}
