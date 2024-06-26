#include <stdlib.h>
#include <stdio.h>
#include "test_memory_5.h"

void kernel(inout_int_t a[4], in_int_t n) {
	for (int i = 2; i < n; i++) {
		a[i] = a[i-1] + a[i-2] + 5;
	}
}

#define AMOUNT_OF_TEST 1

int main(void) {
	inout_int_t a[AMOUNT_OF_TEST][4];
	in_int_t n[AMOUNT_OF_TEST];

	for (int i = 0; i < AMOUNT_OF_TEST; ++i) {
		n[i] = 4;
		for (int j = 0; j < 4; ++j) {
			a[i][j] = rand()%10;
		}
	}
    
	for (int i = 0; i < AMOUNT_OF_TEST; ++i) {
		kernel(a[i], n[i]);
	}

	for (int i = 0; i < AMOUNT_OF_TEST; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%i ", a[i][j]);
		}
	}
}
