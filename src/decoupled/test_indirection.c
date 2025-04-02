#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define TYPE int32_t

TYPE kernel(
        const TYPE *restrict vec,
        const uint32_t *restrict cols,
        uint32_t ncols
) {
    TYPE sum = 0;
    for (uint32_t j = 0; j < ncols; j++) {
        TYPE Si = vec[cols[j]];
        sum += Si;
    }
    return sum;
}

int main() {
    uint32_t ncols = 256;
    TYPE *vec = malloc(sizeof(TYPE) * ncols);
    TYPE ref_result = 0;
    for (uint32_t i = 0; i < ncols; ++i) {
        vec[i] = i;
        ref_result += i;
    }
    uint32_t *cols = malloc(sizeof(uint32_t) * ncols);
    for (uint32_t i = 0; i < ncols; ++i) {
        cols[i] = i;
    }
    TYPE result = kernel(vec, cols, ncols);
    assert(result == ref_result);
    return 0;
}
