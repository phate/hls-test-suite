#include "test_memory_11.h"
#include <stdlib.h>
#include <stdio.h>

void kernel(int a[N]) {
  for (unsigned i = 1; i < N; i++)
    a[i] = a[i - 1] + a[i];
  for (unsigned i = 1; i < N; i++)
    a[i] = a[i - 1];
}

int main(void) {
  inout_int_t a[N];

  srand(13);
  for (unsigned j = 0; j < N; ++j)
    a[j] = (rand() % 100) - 50;

  kernel(a);

  for (unsigned i = 0; i < N; ++i) {
    printf("%i ", a[i]);
  }

  return 0;
}
