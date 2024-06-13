#include "test_memory_12.h"
#include <stdlib.h>
#include <stdio.h>

void kernel(int a[N]) {
  for (unsigned i = 0; i < N; i++) {
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    if (i & 1)
      a[3] = 3;
  }
}

int main(void) {
  inout_int_t a[N];

  srand(13);
  for (unsigned i = 0; i < N; ++i) {
    a[i] = (rand() % 100) - 50;
  }

  kernel(a);

  for (unsigned i = 0; i < N; ++i) {
    printf("%i ", a[i]);
  }

  return 0;
}
