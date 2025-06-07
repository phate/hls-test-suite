#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void kernel(int * a) {
//    int j = *a;
    int i = 0;
    do{
        if(i%2)
            a[i] = i;
        i++;
    } while (i != 10);
}

int main(int argc, char** argv) {
    int a[10] = {0};
	kernel(a);
    for (int i = 0; i < 10; ++i) {
        if(i%2)
            assert(a[i] == i);
        else
            assert(a[i] == 0);
    }
	return 0;
}
