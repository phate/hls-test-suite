#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void kernel(int * a, int cnt) {
    for (u_int32_t i = 0; i < 2; ++i) {
        int sum = 0;
        for (size_t j = 0; j < cnt; ++j) {
            sum += a[j];
        }
        for (size_t j = 0; j < cnt; ++j) {
            a[j] = sum;
        }
    }
}

int main(int argc, char** argv) {
    int a[100] = {1};
	kernel(a, 100);
//    for (int i = 0; i < 5; ++i) {
//        assert(a[i] == i);
//    }
	return 0;
}
