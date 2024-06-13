#include "test_memory_18.h"
#include <stdlib.h>
#include <stdio.h>

void kernel(inout_int_t x[N], in_int_t y[N]) {
  for (unsigned i = 1; i < N; ++i)
    x[i] = x[i] * y[i] + x[0];
}

int main(void) {
  inout_int_t x[N];
  in_int_t y[N];

  for (int j = 0; j < N; ++j) {
    x[j] = rand() % 100;
    y[j] = rand() % 100;
  }

  kernel(x, y);

  for (unsigned i = 0; i < N; ++i) {
    printf("%i, %i ", x[i], y[i]);
  }

  return 0;
}
