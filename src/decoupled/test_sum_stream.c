#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define TYPE float

extern void hls_stream_enq_TYPE(uint32_t channel, TYPE data);
extern TYPE hls_stream_deq_TYPE(uint32_t channel, uint32_t buffer_slots);
enum decoupled_channels{
    sum_dec_channel
};
TYPE kernel(TYPE*  a, uint32_t cnt){
    TYPE result = 0;
    for (uint32_t j = 0; j < cnt; j++) {
        hls_stream_enq_TYPE(sum_dec_channel, a[j]);
    }
    for (uint32_t j = 0; j < cnt; j++) {
        result += hls_stream_deq_TYPE(sum_dec_channel, 10);
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
    assert(result == ref_result);
    free(vec);
    return 0;
}
