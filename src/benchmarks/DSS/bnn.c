void g1(int alpha, int in[10000], int i, int w[10000], int addr_in[10000], int data[10000]){
#pragma SS II=1
	
	int j, x, lut;

	for (j=0;j<100;j++){
		x = i*100+j;
		lut=in[x]^w[x];
		data[addr_in[x]] += lut*alpha;
	} 

}

void g2(int mean[10000], int i, int addr_out[10000], int data[10000]){
#pragma SS II=1
	
	int temp, m, k, y, z;

		for(k=0; k<100; k++){	
		  y=i*100+k;
		  m=mean[y];
		  temp = data[addr_out[y]];
		  if (temp > 0)
			z = temp -m;
		  else
			z = temp +m;
		  data[addr_out[y]] = z;
		}

}

void bnn(int addr_in[10000],int alpha, int w[10000], int addr_out[10000], int data[10000], int in[10000], int mean[10000]){

  int lut, m,i,j,k, x, y, z, temp;

  for (i=0; i<100; i++){
  	g1(alpha, in, i, w, addr_in, data);
	// for (j=0;j<100;j++){
	// 	x = i*100+j;
	// 	lut=in[x]^w[x];
	// 	data[addr_in[x]] += lut*alpha;
	// } 

  	if (i == 99){
  		// An if condition in the loop causes irregular computation.
	    // Static scheduler reserves time slot for each iteration
	    // causing unnecessary pipeline stalls.
  		g2(mean, i, addr_out, data);
		// for(k=0; k<100; k++){
		//   y=i*100+k;
		//   m=mean[y];
		//   temp = data[addr_out[y]];
		//   if (temp > 0)
		// 	z = temp -m;
		//   else
		// 	z = temp +m;
		//   data[addr_out[y]] = z;
		// }
  	 
	}
  }
}

void kernel(int addr_in[10000], int alpha, int w[10000], int addr_out[10000], int data[10000], int in[10000], int mean[10000]){
  int lut, m,i,j,k, x, y, z, temp;

  for (i=0; i<100; i++){
	for (j=0;j<100;j++){
		x = i*100+j;
		lut=in[x]^w[x];
		data[x] += lut*alpha;
	} 

	if (i==99){
		  for(k=0; k<100; k++){
		  y=i*100+k;
		  m=mean[y];
		  temp = data[y];
		  if (temp > 0)
			z = temp -m;
		  else
			z = temp +m;
		  data[y] = z;
		}

	}
  }
}

int main(){

  int addr_in[10000], alpha, w[10000], addr_out[10000], data[10000], gold[10000],in[10000], mean[10000];

  for (int i=0; i<10000; i++){
 	addr_in[i] = i;
	addr_out[i] = i;
	in[i] = 1;
	data[i] = 0;
	gold[i] = 0;
	mean[i] = 1;
	w[i] = 1;
  }

  alpha = 2;

  bnn(addr_in, alpha, w, addr_out, data, in, mean);
  kernel(addr_in, alpha, w, addr_out, gold, in, mean); 

  int count = 0;
  for(int i=0;i<10000; i++)
	count += (data[i] == gold[i]);

  if (count == 10000)
	return 0;
  else
	return 1;

}
