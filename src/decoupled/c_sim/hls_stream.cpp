//
// Created by david on 2/14/25.
//
#include <queue>
#include <cstdint>

#define TYPE float
typedef uint32_t t32v4 __attribute__((vector_size(16)));

#define N_CHANNELS 256

static std::queue<uint32_t> queues_uint32_t[N_CHANNELS];
static std::queue<TYPE> queues_TYPE[N_CHANNELS];
static std::queue<t32v4> queues_t32v4[N_CHANNELS];
extern "C" {
    void hls_stream_enq_uint32t(uint32_t channel, uint32_t data){
        queues_uint32_t[channel].push(data);
    }
    uint32_t hls_stream_deq_uint32t(uint32_t channel, uint32_t buffer_slots){
        uint32_t data = queues_uint32_t[channel].front();
        queues_uint32_t[channel].pop();
        return data;
    }
    void hls_stream_enq_TYPE(uint32_t channel, TYPE data){
        queues_TYPE[channel].push(data);
    }
    TYPE hls_stream_deq_TYPE(uint32_t channel, uint32_t buffer_slots){
        TYPE data = queues_TYPE[channel].front();
        queues_TYPE[channel].pop();
        return data;
    }
    void hls_stream_enq_t32v4(uint32_t channel, t32v4 data){
        queues_t32v4[channel].push(data);
    }
    t32v4 hls_stream_deq_t32v4(uint32_t channel, uint32_t buffer_slots){
        t32v4 data = queues_t32v4[channel].front();
        queues_t32v4[channel].pop();
        return data;
    }
}