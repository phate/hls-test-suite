int g(int d){
#pragma SS II=3
	return (((((((d+112)*d+23)*d+36)*d+82)*d+127)*d+2)*d+20)*d+100;
}

void kernel(int A[1024], int b[1024]) { 
  for (int i = 0; i < 1000; i++) { 
    int d = A[i];
      A[b[i]] = g(d); // need to change the mul to add
  }
}

#include <stdlib.h>

int main(void){
      int A[1024], b[1024];

      for(int j = 0; j < 1024; ++j){
        A[j] = j % 50-25;
        b[j] = rand()%1024;
      }

      kernel(A, b);

      return 0;
}
