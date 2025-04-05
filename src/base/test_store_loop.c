#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void kernel(int * a) {
//    int j = *a;
    int i = 0;
    do{
        a[i] = i;
        i++;
    } while (i != 5);
}

int main(int argc, char** argv) {
    int a[6] = {0};
	kernel(a);
    for (int i = 0; i < 5; ++i) {
        assert(a[i] == i);
    }
	return 0;
}
