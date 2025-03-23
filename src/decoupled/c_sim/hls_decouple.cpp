//
// Created by david on 2/14/25.
//
#include <queue>
#include <cstdint>
#include <cmath>

#define TYPE float
typedef uint32_t t32v4 __attribute__((vector_size(16)));

#define N_CHANNELS 256

#ifndef NO_INSTRUMENTED
extern "C" void reference_load(void* addr, uint64_t width);
extern "C" void reference_store(void* addr, uint64_t width);
#endif

static std::queue<uint32_t> hls_decouple_queues_uint32_t[N_CHANNELS];
static std::queue<TYPE> hls_decouple_queues_TYPE[N_CHANNELS];
static std::queue<t32v4> hls_decouple_queues_t32v4[N_CHANNELS];
extern "C" {
    void hls_decouple_request_32(uint32_t channel, uint32_t * addr){
        uint32_t data = *addr;
        hls_decouple_queues_uint32_t[channel].push(data);
#ifndef NO_INSTRUMENTED
        reference_load(addr, 2);
#endif
    }
    uint32_t hls_decouple_response_32(uint32_t channel, uint32_t buffer_slots){
        uint32_t data = hls_decouple_queues_uint32_t[channel].front();
        hls_decouple_queues_uint32_t[channel].pop();
        return data;
    }
    void hls_decouple_request_TYPE(uint32_t channel, TYPE * addr){
        TYPE data = *addr;
        hls_decouple_queues_TYPE[channel].push(data);
#ifndef NO_INSTRUMENTED
        static_assert(sizeof (TYPE) == 4, "Need to fix referece_load width");
        reference_load(addr, 2);
#endif
    }
    TYPE hls_decouple_response_TYPE(uint32_t channel, uint32_t buffer_slots){
        TYPE data = hls_decouple_queues_TYPE[channel].front();
        hls_decouple_queues_TYPE[channel].pop();
        return data;
    }
    void hls_decouple_request_t32v4(uint32_t channel, t32v4 * addr){
        t32v4 data = *addr;
        hls_decouple_queues_t32v4[channel].push(data);
#ifndef NO_INSTRUMENTED
        reference_load(addr, 4);
#endif
    }
    t32v4 hls_decouple_response_t32v4(uint32_t channel, uint32_t buffer_slots){
        t32v4 data = hls_decouple_queues_t32v4[channel].front();
        hls_decouple_queues_t32v4[channel].pop();
        return data;
    }
}