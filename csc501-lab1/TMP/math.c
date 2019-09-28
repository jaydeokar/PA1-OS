#include <math.h>


double log(double x){

	int n = 1;
	double val = 0.0;

	for (;n<=20;n++){

		val = val - ((pow(x,n))/n);
	}	

	return val;
}

double pow(double x, int y){
	
	int i = 0;
	double prod = 1.0;

	for(;i<y;i++){

		prod = prod*x;
	}

	return prod;
}


double expdev(double lambda){

	double dummy;
    	do{
        	dummy = (double)rand() / 077777; // TODO: should use 077777 or RAND_MAX
    	}
	while (dummy == 0.0);

    	return -(log(dummy) / lambda);

}

