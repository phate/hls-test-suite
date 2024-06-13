void g(int data[200], int j, int w[10000]){
  int i;
  for (i=0; i<100; i++){
    data[i+100] += w[i*100+j]*data[i];
  } 
}

void kernel(int data[200], int all_zero[100], int w[10000]){
  int j, i, temp;

  for (j=0; j<100; j++){

    temp = all_zero[j];
    if (temp < 10){
    	// An if condition in the loop causes irregular computation.
	// Static scheduler reserves time slot for each iteration
	// causing unnecessary pipeline stalls.
    	g(data, j, w);
		// for (i=0; i<100; i++){
		//      data[i+100] += w[i*100+j]*data[i];
		// } 
    }
  }
}

int main(){
  int data[200], az[100], w[10000], data1[200];
  int temp;

  for(int i=0; i<100; i++){
    data[i] = 1;
    data[i+100] = 0;
    data1[i] = 1;
    data1[i+100] = 0;
    az[i] = i%5;
  }

  for(int i=0; i<10000; i++){
    w[i] = i%10;
  }

  kernel(data, az, w);

  for (int j=0; j<100; j++){
    temp = az[j];
    if (temp < 10){
      for (int i=0; i<100; i++){
        data1[i+100] += w[i*100+j]*data[i];
      } 
    }
  }

  int res = 0;
  for(int i = 0; i< 200; i++)
    res += (data[i] == data1[i]);

  if (res == 200)
    return 0;
  else {
    return 1;
  }

}
