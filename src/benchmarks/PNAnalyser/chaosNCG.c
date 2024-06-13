void g(int b0, int b1, int p0, int p1, int X, int Y, int *pp0, int *pp1, int *out0, int *out1){
#pragma SS II=2
  p0 ^= b0, p1 ^= b1;
  p1 ^= (Y << (p0 % 17) | Y >> (16 - p0 % 17));
  p0 ^= p1;
  p1 += (X & p0);
  *out0 = b0 - p1;
  p0 += (X << (p1 % 17) | X >> (16 - p1 % 17));
  p1 += p0;
  p0 += (Y & p1);
  *out1 = b1 + p0;
  *pp0 = p0;
  *pp1 = p1;
}

void kernel(int I, int bo, int M[2000], int X, int Y, int params0, int params1, int buffer[2000]){
	int bound = 2*bo;
	int p0 = params0;
	int p1 = params1;
	for (int i = 0; i < bound; i += 2) {
// #pragma HLS PIPELINE - vhls - 78
	  int a = M[I+i-2], b = M[I+i+2];
	  int b0 = buffer[a], b1 = buffer[b];
    int pp0, pp1, out0, out1;
      g(b0, b1, p0, p1, X, Y, &pp0, &pp1, &out0, &out1);
//    p0 ^= b0, p1 ^= b1;
//	  p1 ^= (Y << (p0 % 17) | Y >> (16 - p0 % 17));
//	  p0 ^= p1;
//	  p1 += (X & p0);
	  buffer[a] = out0;
//	  p0 += (X << (p1 % 17) | X >> (16 - p1 % 17));
//	  p1 += p0;
//	  p0 += (Y & p1);
      p0 = pp0;
      p1 = pp1;
	  buffer[b] = out1;
	}
}

#include <stdlib.h>

int main(){
	int I = 10, bo = 900, M[2000], X = 250, Y = 250, params[2], buffer[2000];
	params[0] = 127;
	params[1] = 41;
	for (int i = 0; i < 2000; i++){
		M[i] = rand()%2000;
		buffer[i] = rand()%2000;
	}

	kernel(I, bo, M, X, Y, 127, 41, buffer);
	return 0;
}

