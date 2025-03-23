#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

#define TYPE int32_t



typedef uint32_t bucket_vector __attribute__((vector_size(16)));

extern void hls_decouple_request_t32v4(uint32_t channel, bucket_vector * addr);
extern bucket_vector hls_decouple_response_t32v4(uint32_t channel, uint32_t buffer_slots);

extern void hls_decouple_request_32(uint32_t channel, uint32_t * addr);
extern uint32_t hls_decouple_response_32(uint32_t channel, uint32_t buffer_slots);

extern void hls_stream_enq_uint32t(uint32_t channel, uint32_t data);
extern uint32_t hls_stream_deq_uint32t(uint32_t channel, uint32_t buffer_slots);

enum decoupled_channels{
    key_channel,
    bucket_channel,
    i_stream,
    key_stream,
    i_stream2,
    key_stream2
};

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <malloc.h>

const int NUM = 10;
#define TYPE int32_t
#define MIN(a, b) ((a)>(b)?(b):(a))

typedef struct {
    uint32_t key;
    uint32_t id;
    uint32_t next_idx;
    uint32_t next_valid;
} tBucket;


void kernel(
        const uint32_t * table,
        const bucket_vector * hashtable,
        uint32_t * result,
        uint32_t tableElements,
        uint32_t hashMask
) {
#define OUTSTANDING_READS 128

    for (uint32_t i = 0; i < tableElements; ++i) {
        hls_decouple_request_32(key_channel, &table[i]);
    }

    uint32_t rif = 0;
    uint32_t key_i = 0;
    for (uint32_t j = 0; j < tableElements;){
        uint32_t key;
        uint32_t i;
        uint32_t idx;
        if (rif < OUTSTANDING_READS && key_i < tableElements){
            i = key_i;
            key = hls_decouple_response_32(key_channel, OUTSTANDING_READS);
            key_i++;
            idx = key & hashMask;
        } else if(rif) {
            i = hls_stream_deq_uint32t(i_stream2, OUTSTANDING_READS);
            key = hls_stream_deq_uint32t(key_stream2, OUTSTANDING_READS);
            bucket_vector b = hls_decouple_response_t32v4(bucket_channel, OUTSTANDING_READS);
            rif--;
            if (key == b[0]) {
                result[i] = b[1];
                j++;
                continue;
            } else if (b[3]) {
                idx = b[2];
            } else {
                result[i] = 0;
                j++;
                continue;
            }
        } else {
            continue;
        }
        hls_decouple_request_t32v4(bucket_channel, &hashtable[idx]);
        hls_stream_enq_uint32t(i_stream2, i);
        hls_stream_enq_uint32t(key_stream2, key);
        rif++;
    }
}

void * allocate(size_t size){
    return memalign(4096, size+4096);
}

#define HPRIME 0x1234566
//0xBIG
#define MASK 0xFFFF
#define HASH(X) (((X) & MASK) ^ HPRIME)
#define MIN(a, b) ((a)>(b)?(b):(a))

void hashtable_reference(
        const uint32_t * table,
        const tBucket * hashtable,
        uint32_t * result,
        uint32_t tableElements,
        uint32_t hashMask
) {
    uint32_t pointers_chased = 0;
    uint32_t not_found = 0;
    uint32_t max_chain_length = 0;

    for (uint32_t i = 0; i < tableElements; ++i) {
        uint32_t key = table[i];
        uint32_t idx = key & hashMask;
        tBucket b = hashtable[idx];
        uint32_t chain_length = 1;
        pointers_chased++;
        while (1){
            if (key == b.key) {
                result[i] = b.id;
                if(chain_length>max_chain_length){
                    max_chain_length = chain_length;
                }
                break;
            } else if (b.next_valid) {
                b = hashtable[b.next_idx];
                chain_length++;
                pointers_chased++;
            } else {
                // not found in hash-map
                result[i] = 0;
                not_found++;
                break;
            }
        }
    }
    printf("table elements: %d, pointers chased: %d, not found: %d, avg chain length: %.2f, max chain length: %d\n", tableElements, pointers_chased, not_found, ((float)pointers_chased)/tableElements, max_chain_length);
}

void test_hashtable(uint32_t hashBits, uint32_t hashEntries, uint32_t tableEntries) {
    uint32_t hashMask = (1 << hashBits) - 1;

    assert(hashEntries>tableEntries);

    tBucket *hashTable = (tBucket*) allocate((hashMask+hashEntries)*sizeof(tBucket));
    uint32_t *table = (uint32_t*) allocate(sizeof(TYPE) * tableEntries);
    uint32_t *result = (uint32_t*) allocate(sizeof(TYPE) * tableEntries);
    uint32_t *result_ref = (uint32_t*) allocate(sizeof(TYPE) * tableEntries);
    uint32_t *expected_result = (uint32_t*) allocate(sizeof(TYPE) * tableEntries);
    uint32_t overflowIdx = hashMask+1;
    for (uint32_t i = 0; i < hashEntries; ++i) {
        uint32_t key = random();
        uint32_t id = random();
        uint32_t hash = key & hashMask;
        tBucket * bucket = &hashTable[hash];
        tBucket * previous = NULL;
        // handle duplicate keys
        while(!((bucket->key == 0 && bucket->id == 0)||bucket->key == key)){
            previous = bucket;
            if(bucket->next_valid){
                bucket = &hashTable[bucket->next_idx];
            } else {
                bucket = &hashTable[overflowIdx];
                assert(bucket->key == 0 && bucket->id == 0);
            }
        }
        if(previous && bucket->key != key){
            assert(!previous->next_valid);
            previous->next_valid = 1;
            previous->next_idx = overflowIdx;
            overflowIdx++;
        }
        bucket->key = key;
        bucket->id = id;

        uint32_t j;
        if(i<tableEntries){
            // fill up table first
            j = i;
        } else {
            // then replace random entries
            j = random() % tableEntries;
        }
        table[j] = key;
        expected_result[j] = id;
    }

    uint32_t hashTableSize = overflowIdx;

    hashtable_reference(table, hashTable, result_ref, tableEntries, hashMask);
    kernel(table, hashTable, result, tableEntries, hashMask);

    for (uint32_t i = 0; i < tableEntries; ++i) {
        assert(result[i] == expected_result[i]);
    }

    free(hashTable);
    free(table);
    free(result);
    free(result_ref);
    free(expected_result);
}

int main() {
    test_hashtable(8, 1<<12, 1024);
    return 0;
}
