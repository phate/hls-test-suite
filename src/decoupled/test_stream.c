#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

extern void hls_stream_enq_uint32t(uint32_t channel, int data);
extern uint32_t hls_stream_deq_uint32t(uint32_t channel, uint32_t buffer_slots);

enum stream_channels{
    stream_channel0,
};

int kernel(int a, int b) {
	for (int i=0; i<b; i++) {
		a++;
        hls_stream_enq_uint32t(stream_channel0, a);
	}
    int c;
	for (int i=0; i<b; i++) {
		c = hls_stream_deq_uint32t(stream_channel0, 5);
	}
	return c;
}

int main(int argc, char** argv) {
	int result = kernel(1, 5);
	printf("Result: %i\n", result);

	// Check if correct result
	if (result == 6)
		return 0;

	return 1;
}
