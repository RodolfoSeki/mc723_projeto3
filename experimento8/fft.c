/* 
 * Free FFT and convolution (C)
 * 
 * Copyright (c) 2014 Project Nayuki
 * https://www.nayuki.io/page/free-small-fft-in-multiple-languages
 * 
 * (MIT License)
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "fft.h"

#define SIN_ADDRESS 0x6500001
#define COS_ADDRESS 0x6500000
#define SUM_ADDRESS 0x6600000
#define SUB_ADDRESS 0x6700000
#define MUL_ADDRESS 0x6800000
#define DIV_ADDRESS 0x6900000
#define WRITE_A     0x6700000
#define WRITE_B     0x6700001
#define LOCK_ADDRESS 0x6400000
#define NUMBER_OF_CORES 2


volatile int *lock = (volatile int *) LOCK_ADDRESS;

volatile int *sin_op = (volatile int *)  SIN_ADDRESS;
volatile int *cos_op = (volatile int *)  COS_ADDRESS;

volatile int *sum_op = (volatile int *)  SUM_ADDRESS;
volatile int *sub_op = (volatile int *)  SUB_ADDRESS;
volatile int *mul_op = (volatile int *)  MUL_ADDRESS;
volatile int *div_op = (volatile int *)  DIV_ADDRESS;

volatile int *write_a = (volatile int *) WRITE_A;
volatile int *write_b = (volatile int *) WRITE_B;

volatile int firstSemaphore = 0;
volatile int secondSemaphore = 0;
volatile int thirdSemaphore = 0;
volatile int fourthSemaphore = 0;
volatile int fifthSemaphore = 0;

volatile float areal[130];
volatile float aimag[130];
volatile float dreal[130];
volatile float dimag[130];


void AcquireLock() {
        while(*lock);
}
void ReleaseLock(){
        *lock = 0;
}

void incrementSemaphore(volatile int* s){
        AcquireLock();
        (*s)++;
        //if (*s > NUMBER_OF_CORES)
        //      (*s) = 1;
        ReleaseLock();
}

void acquireSemaphore(volatile int* s){
        while((*s) < NUMBER_OF_CORES);
}

void whoAmI(int procNumber){
        AcquireLock();
        printf("procNumber = %d\n", procNumber);
        ReleaseLock();
}

void getVectorParcel(int * start, int * end, int size, int procNumber){
        *start = procNumber*(size/NUMBER_OF_CORES);
        *end = (procNumber+1)*(size/NUMBER_OF_CORES);
        if(procNumber == NUMBER_OF_CORES-1){
                //se tem resto, tem que colocar na ultima parcela
                *end += size%NUMBER_OF_CORES;
        }
}


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

float cos_mod_two(float k, int n){
	return cos_acc(div_acc(mul_acc(mul_acc(2, M_PI), k), n));
}

float sin_mod_two(float k, int n){
	return sin_acc(div_acc(mul_acc(mul_acc(2, M_PI), k), n));
}

float cos_mod(float i, int n){
	return cos_acc(div_acc(mul_acc(mul_acc(M_PI, i), i), n));
}

float sin_mod(float i, int n){
	return sin_acc(div_acc(mul_acc(mul_acc(M_PI, i), i), n));
}

// Private function prototypes
static size_t reverse_bits(size_t x, unsigned int n);
static void *memdup(const void *src, size_t n);

#define N_MAX ((size_t)-1)


int transform(float real[], float imag[], size_t n, int procNum) {
	if (n == 0)
		return 1;
	else if ((n & (n - 1)) == 0)  // Is power of 2
		return transform_radix2(real, imag, n, procNum);
	else  // More complicated algorithm for arbitrary sizes
		return transform_bluestein(real, imag, n, procNum);
}


int inverse_transform(float real[], float imag[], size_t n, int procNum) {
	return transform(imag, real, n, procNum);
}


int transform_radix2(float real[], float imag[], size_t n, int procNum) {
	// Variables
	int status = 0;
	unsigned int levels;
	//float *cos_table, *sin_table;
	size_t size;
	size_t i;
	
	// Compute levels = floor(log2(n))
	{
		size_t temp = n;
		levels = 0;
		while (temp > 1) {
			levels++;
			temp >>= 1;
		}
		if (1u << levels != n)
			return 0;  // n is not a power of 2
	}
	
	// Trignometric tables
	if (N_MAX / sizeof(float) < n / 2)
		return 0;
	/*size = (n / 2) * sizeof(float);
	cos_table = malloc(size);
	sin_table = malloc(size);
	if (cos_table == NULL || sin_table == NULL)
		goto cleanup;
	for (i = 0; i < n / 2; i++) {
		cos_table[i] = cos(2 * M_PI * i / n);
		sin_table[i] = sin(2 * M_PI * i / n);
	}*/
	
	// Bit-reversed addressing permutation
	for (i = 0; i < n; i++) {
		size_t j = reverse_bits(i, levels);
		if (j > i) {
			float temp = real[i];
			real[i] = real[j];
			real[j] = temp;
			temp = imag[i];
			imag[i] = imag[j];
			imag[j] = temp;
		}
	}
	
	// Cooley-Tukey decimation-in-time radix-2 FFT
	for (size = 2; size <= n; size *= 2) {
		size_t halfsize = size / 2;
		size_t tablestep = n / size;
		for (i = 0; i < n; i += size) {
			size_t j;
			size_t k;
			for (j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
				float tpre = sum_acc(mul_acc(real[j+halfsize], cos_mod_two(k,n)), mul_acc(imag[j+halfsize], sin_mod_two(k,n)));
				float tpim = sum_acc(mul_acc(-real[j+halfsize], sin_mod_two(k,n)), mul_acc(imag[j+halfsize], cos_mod_two(k,n)));
				real[j + halfsize] = sub_acc(real[j], tpre);
				imag[j + halfsize] = sub_acc(imag[j], tpim);
				real[j] = sum_acc(real[j],tpre);
				imag[j] = sum_acc(imag[j],tpim);
			}
		}
		if (size == n)  // Prevent overflow in 'size *= 2'
			break;
	}
	status = 1;
	
	
	
cleanup:
	//free(cos_table);
	//free(sin_table);
	return status;
}

int transform_bluestein(float real[], float imag[], size_t n, int procNumber) {

 // Variables
        int status = 0;
        //float *cos_table, *sin_table;
        //float *areal, *aimag;
        float *breal, *bimag;
        //float *dreal, *dimag;

	size_t m;
	size_t size_n, size_m;
	size_t i;
	
	// Find a power-of-2 convolution length m such that m >= n * 2 + 1
	{
		size_t target;
		if (n > (N_MAX - 1) / 2)
			return 0;
		target = n * 2 + 1;
		for (m = 1; m < target; m *= 2) {
			if (N_MAX / 2 < m)
				return 0;
		}
	}
	
	// Allocate memory
	if (N_MAX / sizeof(float) < n || N_MAX / sizeof(float) < m)
		return 0;
	size_n = n * sizeof(float);
	size_m = m * sizeof(float);
	breal = calloc(m, sizeof(float));
	bimag = calloc(m, sizeof(float));

	// Zera os vetores a e b
	int start, end;
        getVectorParcel(&start,&end, n, procNumber);
        for (i = start; i < end; i++) {
                areal[i] = 0;
                aimag[i] = 0;
        }
        incrementSemaphore(&firstSemaphore);
        acquireSemaphore(&firstSemaphore);

	
	// Temporary vectors and preprocessing
	getVectorParcel(&start,&end, n, procNumber);
        for (i = start; i < end; i++) {
		areal[i] = sum_acc(mul_acc(real[i], cos_mod(i,n)), mul_acc(imag[i], sin_mod(i,n)));
		aimag[i] = sum_acc(mul_acc(-real[i], sin_mod(i,n)), mul_acc(imag[i], cos_mod(i,n)));
	}
        incrementSemaphore(&secondSemaphore);
        acquireSemaphore(&secondSemaphore);

	breal[0] = cos_mod(0,n);
	bimag[0] = sin_mod(0,n);
	for (i = 1; i < n; i++) {
		breal[i] = breal[m - i] = cos_mod(i,n);
		bimag[i] = bimag[m - i] = sin_mod(i,n);
	}
	
	// Convolution
	if (!convolve_complex(breal, bimag, m, procNumber))
		goto cleanup;
	
	// Postprocessing
	for (i = 0; i < n; i++) {
		real[i] = sum_acc(mul_acc(dreal[i], cos_mod(i,n)), mul_acc(dimag[i], sin_mod(i,n)));
		imag[i] = sum_acc(mul_acc(-dreal[i], sin_mod(i,n)), mul_acc(dimag[i], cos_mod(i,n)));
	}
	status = 1;
	
	// Deallocation
cleanup:
	free(bimag);
	free(breal);
	return status;
}


int convolve_complex(float* breal, float* bimag, size_t n, int procNumber) {

	int status = 0;
	size_t size;
	size_t i;
	float *xr, *xi, *yr, *yi;
	int start, end;
	if (N_MAX / sizeof(float) < n)
		return 0;
	size = n * sizeof(float);
	xr = memdup(areal, size);
	xi = memdup(aimag, size);
	yr = memdup(breal, size);
	yi = memdup(bimag, size);
	if (xr == NULL || xi == NULL || yr == NULL || yi == NULL)
		goto cleanup;
	
	if (!transform(xr, xi, n, procNumber))
		goto cleanup;
	if (!transform(yr, yi, n, procNumber))
		goto cleanup;

	for (i = 0; i < n; i++) {
		float temp = sub_acc(mul_acc(xr[i], yr[i]), mul_acc(xi[i], yi[i]));
		xi[i] = sum_acc(mul_acc(xi[i], yr[i]), mul_acc(xr[i], yi[i]));
		xr[i] = temp;
	}
	if (!inverse_transform(xr, xi, n, procNumber))
		goto cleanup;

       	getVectorParcel(&start,&end, n, procNumber);
        for (i = start; i < end; i++) {
		dreal[i] = div_acc(xr[i], n);
		dimag[i] = div_acc(xi[i], n);
	}
	status = 1;
	incrementSemaphore(&fifthSemaphore);
        acquireSemaphore(&fifthSemaphore);

	
cleanup:
	free(yi);
	free(yr);
	free(xi);
	free(xr);
	return status;
}

static size_t reverse_bits(size_t x, unsigned int n) {
        size_t result = 0;
        unsigned int i;
        for (i = 0; i < n; i++, x >>= 1)
                result = (result << 1) | (x & 1);
        return result;
}

static void *memdup(const void *src, size_t n) {
        void *dest = malloc(n);
        if (dest != NULL)
                memcpy(dest, src, n);
        return dest;
}
