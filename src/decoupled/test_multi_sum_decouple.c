#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define TYPE float

#define LATENCY 100
#define ITERATIONS 10

extern void hls_decouple_request_TYPE(uint32_t channel, const TYPE * addr);
extern TYPE hls_decouple_response_TYPE(uint32_t channel, uint32_t buffer_slots);
enum decoupled_channels{
    sum_dec_channel
};
TYPE kernel(TYPE*  a, uint32_t cnt){
    TYPE result = 0;
    for (uint32_t k = 0; k < ITERATIONS; ++k) {
        for (uint32_t j = 0; j < cnt; j++) {
            hls_decouple_request_TYPE(sum_dec_channel, &a[j]);
        }
        for (uint32_t j = 0; j < cnt; j++) {
            result += hls_decouple_response_TYPE(sum_dec_channel, LATENCY);
        }
    }
	return result;
}

int main(int argc, char** argv){
    uint32_t cnt = 128;
    TYPE *vec = malloc(sizeof(TYPE) * cnt);
    TYPE ref_result = 0;
    for (uint32_t i = 0; i < cnt; ++i) {
        vec[i] = i;
        ref_result += i;
    }
    TYPE result = kernel(vec, cnt);
    assert(result == ref_result*ITERATIONS);
    free(vec);
    return 0;
}
