#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<stdint.h>

#define LOCK_ADDRESS 0x6400000;

#define SIN_ADDRESS 0x6500001
#define COS_ADDRESS 0x6500000

#define SUM_ADDRESS 0x6600000
#define SUB_ADDRESS 0x6700000
#define MUL_ADDRESS 0x6800000
#define DIV_ADDRESS 0x6900000

#define WRITE_A     0x6700000
#define WRITE_B     0x6700001

volatile int procCounter = 0;

volatile int *lock = (volatile int *) LOCK_ADDRESS;
volatile int *sin_op = (volatile int *)  SIN_ADDRESS;
volatile int *cos_op = (volatile int *)  COS_ADDRESS;
volatile int *sum_op = (volatile int *)  SUM_ADDRESS;
volatile int *sub_op = (volatile int *)  SUB_ADDRESS;
volatile int *mul_op = (volatile int *)  MUL_ADDRESS;
volatile int *div_op = (volatile int *)  DIV_ADDRESS;
volatile int *write_a = (volatile int *) WRITE_A;
volatile int *write_b = (volatile int *) WRITE_B;


float sin_acc(float value){
        *sin_op = *((uint32_t*)&value);
        return *((float*)&(*sin_op));
}
float cos_acc(float value){
        *cos_op = *((uint32_t*)&value);
        return *((float*)&(*cos_op));
}

float sum_acc (float value_a, float value_b){
        (*write_a) = *((uint32_t*)&value_a);
        (*write_b) = *((uint32_t*)&value_b);
        return *((float*)&(*sum_op));
}

float sub_acc(float value_a, float value_b){
        (*write_a) = *((uint32_t*)&value_a);
        (*write_b) = *((uint32_t*)&value_b);
        return *((float*)&(*sub_op));
}
float mul_acc(float value_a, float value_b){
        (*write_a) = *((uint32_t*)&value_a);
        (*write_b) = *((uint32_t*)&value_b);
        return *((float*)&(*mul_op));
}
float div_acc(float value_a, float value_b){
        (*write_a) = *((uint32_t*)&value_a);
        (*write_b) = *((uint32_t*)&value_b);
        return *((float*)&(*div_op));
}

int main( int ac, char *av[]) {

	printf("Testa Lock\n");
	printf("resultado | esperado\n");
	printf("lock %d | 0\n", (*lock));
	printf("locked %d | 1\n", (*lock));
	printf("unlock\n");
	*lock = 1;
	printf("unlocked %d | 0\n", (*lock));
	*lock = 0;

	srand(time(NULL));

	printf("Testa float\n");
	int i;
	float a, b;

	printf("resultado | esperado\n");
	for (i = 0; i < 4; i++) {
		a = (float) rand()/RAND_MAX * 100;
		b = (float) rand()/RAND_MAX * 100;
		printf("%f + %f = %f | %f\n", a, b, sum_acc(a,b), a + b);
		printf("%f - %f = %f | %f\n", a, b, sub_acc(a,b), a - b);
		printf("%f * %f = %f | %f\n", a, b, mul_acc(a,b), a * b);
		printf("%f / %f = %f | %f\n", a, b, div_acc(a,b), a / b);
	}
	
	printf("Testa seno e coseno\n");
	printf("resultado | esperado\n");
	for (i = 0; i < 4; i++) {
		a = (float) rand()/RAND_MAX * 100;
		printf("sen(%f) = %f | %f\n", a, sin_acc(a), sin(a));
		printf("cos(%f) = %f | %f\n", a, cos_acc(a), cos(a));
	}

	return 0;
}
