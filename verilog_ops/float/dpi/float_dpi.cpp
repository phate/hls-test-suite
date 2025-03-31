//
// Created by david on 3/23/25.
//
#include <stdint.h>
extern "C" {
// DPI-C function to add two 32-bit floats
void float_add_dpi_c(const uint32_t *a, const uint32_t *b, uint32_t *out) {
    // Reinterpret the 32-bit integers as floats
    float fa = *(float *) a;
    float fb = *(float *) b;
    float result = fa + fb;

    // Reinterpret the result back to a 32-bit integer
    *out = *(uint32_t *) &result;
}

// DPI-C function to multiply two 32-bit floats
void float_mul_dpi_c(const uint32_t *a, const uint32_t *b, uint32_t *out) {
    // Reinterpret the 32-bit integers as floats
    float fa = *(float *) a;
    float fb = *(float *) b;
    float result = fa * fb;

    // Reinterpret the result back to a 32-bit integer
    *out = *(uint32_t *) &result;
}
}