#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define TYPE float

#define LATENCY 100

extern void hls_decouple_request_32(uint32_t channel, const uint32_t * addr);
extern uint32_t hls_decouple_response_32(uint32_t channel, uint32_t buffer_slots);

extern void hls_decouple_request_TYPE(uint32_t channel, const TYPE * addr);
extern TYPE hls_decouple_response_TYPE(uint32_t channel, uint32_t buffer_slots);
enum decoupled_channels{
    cols_dec_channel,
    vec_dec_channel
};

TYPE kernel(
        const TYPE *restrict vec,
        const uint32_t *restrict cols,
        uint32_t ncols
) {
    TYPE sum = 0;
    for (uint32_t j = 0; j < ncols; j++) {
        hls_decouple_request_32(cols_dec_channel, &cols[j]);
    }
    for (uint32_t j = 0; j < ncols; j++) {
        uint32_t i = hls_decouple_response_32(cols_dec_channel, LATENCY);
        hls_decouple_request_TYPE(vec_dec_channel, &vec[i]);
    }
    for (uint32_t j = 0; j < ncols; j++) {
        TYPE Si = hls_decouple_response_TYPE(vec_dec_channel, LATENCY);
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
