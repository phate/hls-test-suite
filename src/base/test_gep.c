#include <stdlib.h>
#include <assert.h>

int kernel(int C[ 20 + 0][25 + 0], int i, int j){
    return C[i][j];
}

int main(int argc, char** argv){
    int (*C)[20 + 0][25 + 0];
    C = (int(*)[20 + 0][25 + 0])calloc ((20 + 0) * (25 + 0), sizeof(int));

    (*C)[11][7] = 1;

    assert(kernel(*C, 5, 13) == 0);

    assert(kernel(*C, 7, 11) == 0);

    assert(kernel(*C, 11, 7) == 1);

    free((void*)C);
    return 0;
}
