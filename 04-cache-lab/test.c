

#include <stdio.h>



void swap(int *a, int *b){

	int tmp = *a;
	*a=*b;
	*b=tmp;

}


void change(short *evicted){

*evicted =  (*evicted) ^ 1;

}

int main(){

int m = 10, n = 20;

swap(&m, &n);

printf("m = %d, n = %d\n", m , n );


short evicted = 0;

change(&evicted);

printf("e1: %i\n", evicted );

change(&evicted);

printf("e2: %i\n", evicted );

change(&evicted);

printf("e3: %i\n", evicted );



}
