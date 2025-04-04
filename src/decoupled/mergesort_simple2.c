#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <malloc.h>

#define MIN(a, b) ((a)>(b)?(b):(a))
void kernel(
        uint32_t *table,
        uint32_t *result,
        uint32_t n
) {
    uint32_t width2;
    for (uint32_t width = 1; width < n; width = width2) {
        width2 = width << 1;
        for (uint32_t i_outer = 0; i_outer < n; i_outer+=width2) {
            uint32_t i_left = i_outer;
            uint32_t i_right = MIN(i_outer + width, n);
            uint32_t i_end = MIN(i_outer + width2, n);
            uint32_t i = i_left;
            uint32_t j = i_right;
            uint32_t table_i;
            uint32_t table_j;
            bool update_table_i = true;
            bool update_table_j = true;
            for (uint32_t k = i_left; k < i_end; ++k) {
                if(update_table_i){
                    table_i = table[i];
                }
                update_table_i = false;
                if(update_table_j){
                    table_j = table[j];
                }
                update_table_j = false;
                if (i < i_right && (j >= i_end || table_i <= table_j)) {
                    result[k] = table_i;
                    update_table_i = true;
                    i = i + 1;
                } else {
                    result[k] = table_j;
                    update_table_j = true;
                    j = j + 1;
                }
            }
        }
        if(width2<n){
            for (uint32_t i = 0; i < n; ++i) {
                table[i] = result[i];
            }
        }
    }
}

int comp(const void *elem1, const void *elem2) {
    int f = *((uint32_t *) elem1);
    int s = *((uint32_t *) elem2);
    if (f > s) return 1;
    if (f < s) return -1;
    return 0;
}

void sort_ref(uint32_t *arr, uint32_t size) {
    qsort(arr, size, sizeof(uint32_t), comp);
}

void *allocate(size_t size) {
    return memalign(4096, size + 4096);
}

int main() {
    srand(0);
    uint32_t sort_elements = 234;
    uint32_t *table = allocate(sort_elements * sizeof(uint32_t));
    uint32_t *result = allocate(sort_elements * sizeof(uint32_t));
    uint32_t *expected_result = allocate(sort_elements * sizeof(uint32_t));
    for (size_t i = 0; i < sort_elements; ++i) {
        uint32_t tmp = rand();
        table[i] = tmp;
        expected_result[i] = tmp;
    }
    sort_ref(expected_result, sort_elements);
    kernel(table, result, sort_elements);
    uint32_t prev_element = 0;
    for (size_t i = 0; i < sort_elements; ++i) {
        assert(result[i] == expected_result[i]);
        assert(expected_result[i] >= prev_element);
        prev_element = expected_result[i];
    }
}
