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
#define NUMBER_OF_CORES 8

volatile float *sin_op=(float *)  SIN_ADDRESS;
volatile float *cos_op=(float *)  COS_ADDRESS;
volatile float *sum_op=(float *)  SUM_ADDRESS;
volatile float *sub_op=(float *)  SUB_ADDRESS;
volatile float *mul_op=(float *)  MUL_ADDRESS;
volatile float *div_op=(float *)  DIV_ADDRESS;
volatile float *write_a=(float *) WRITE_A;
volatile float *write_b=(float *) WRITE_B;

volatile int *lock = (int *) LOCK_ADDRESS;
volatile int firstSemaphore = 0;
volatile int secondSemaphore = 0;
volatile int thirdSemaphore = 0;
volatile int fourthSemaphore = 0;
volatile int fifthSemaphore = 0;

volatile float areal[130];
volatile float aimag[130];
//volatile float breal[130];
//volatile float bimag[130];
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
	//	(*s) = 1;
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
	*sin_op = value;
	return *sin_op;
}
float cos_acc(float value){
	*cos_op = value;
	return *cos_op;
}

float sum_acc(float value_a, float value_b){
	*write_a = value_a;
	*write_b = value_b;
	return *sum_op;
}
float sub_acc(float value_a, float value_b){
	*write_a = value_a;
	*write_b = value_b;
	return *sub_op;
}
float mul_acc(float value_a, float value_b){
	*write_a = value_a;
	*write_b = value_b;
	return *mul_op;
}
float div_acc(float value_a, float value_b){
	*write_a = value_a;
	*write_b = value_b;
	return *div_op;
}

float cos_mod_two(float k, int n){
	return cos(2 * M_PI * k / n);
}

float sin_mod_two(float k, int n){
	return sin(2 * M_PI * k / n);
}

float cos_mod(float i, int n){
	return cos( M_PI * i * i / n);
}

float sin_mod(float i, int n){
	return sin( M_PI * i * i / n);
}

// Private function prototypes
static size_t reverse_bits(size_t x, unsigned int n);
static void *memdup(const void *src, size_t n);

#define SIZE_MAX ((size_t)-1)


int transform(float real[], float imag[], size_t n, int procNumber) {
	if (n == 0)
		return 1;
	else if ((n & (n - 1)) == 0)  // Is power of 2
		return transform_radix2(real, imag, n, procNumber);
	else  // More complicated algorithm for arbitrary sizes
		return transform_bluestein(real, imag, n, procNumber);
}


int inverse_transform(float real[], float imag[], size_t n, int procNumber) {
	return transform(imag, real, n, procNumber);
}


int transform_radix2(float real[], float imag[], size_t n, int procNumber) {
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
	if (SIZE_MAX / sizeof(float) < n / 2)
		return 0;

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
                                float tpre =  real[j+halfsize] * cos_mod_two(k,n) + imag[j+halfsize] * sin_mod_two(k,n);
                                float tpim = -real[j+halfsize] * sin_mod_two(k,n) + imag[j+halfsize] * cos_mod_two(k,n);
                                real[j + halfsize] = real[j] - tpre;
                                imag[j + halfsize] = imag[j] - tpim;
                                real[j] += tpre;
                                imag[j] += tpim;
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
		if (n > (SIZE_MAX - 1) / 2)
			return 0;
		target = n * 2 + 1;
		for (m = 1; m < target; m *= 2) {
			if (SIZE_MAX / 2 < m)
				return 0;
		}
	}

	// Allocate memory
	if (SIZE_MAX / sizeof(float) < n || SIZE_MAX / sizeof(float) < m)
		return 0;
	size_n = n * sizeof(float);
	size_m = m * sizeof(float);

        breal = calloc(m, sizeof(float));
        bimag = calloc(m, sizeof(float));


	// Zera os vetores a e b
	int start, end;
	getVectorParcel(&start,&end, n, procNumber);
	for (i = start; i < end; i++) {
	//for (i = 0; i < n; i++) {
		areal[i] = 0;
		aimag[i] = 0;
	}
	incrementSemaphore(&firstSemaphore);
	acquireSemaphore(&firstSemaphore);

	// Temporary vectors and preprocessing
	getVectorParcel(&start,&end, n, procNumber);
	for (i = start; i < end; i++) {
	//AcquireLock();
	//for (i = 0; i < n; i++) {
		areal[i] =  real[i] * cos_mod(i,n) + imag[i] * sin_mod(i,n);
		aimag[i] = -real[i] * sin_mod(i,n) + imag[i] * cos_mod(i,n);
	}
	//ReleaseLock();
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
	//getVectorParcel(&start,&end, n, procNumber);
	//for (i = start; i < end; i++) {
	for (i = 0; i < n; i++) {
		real[i] =  dreal[i] * cos_mod(i,n) + dimag[i] * sin_mod(i,n);
		imag[i] = -dreal[i] * sin_mod(i,n) + dimag[i] * cos_mod(i,n);
	}
	//incrementSemaphore(&thirdSemaphore);
	//acquireSemaphore(&thirdSemaphore);
	status = 1;

	// Deallocation
cleanup:
	//free(dimag);
	//free(dreal);
	free(bimag);
	free(breal);
	//free(aimag);
	//free(areal);
	return status;
}



int convolve_complex(float* breal, float* bimag, size_t n, int procNumber) {
	int status = 0;
	size_t size;
	size_t i;
	float *xr, *xi, *yr, *yi;
	if (SIZE_MAX / sizeof(float) < n)
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


	int start, end;
	//getVectorParcel(&start,&end, n, procNumber);
	//for (i = start; i < end; i++) {
	for (i = 0; i < n; i++) {
		float temp = xr[i] * yr[i] - xi[i] * yi[i];
		xi[i] = xi[i] * yr[i] + xr[i] * yi[i];
		xr[i] = temp;
	}
	//incrementSemaphore(&fourthSemaphore);
	//acquireSemaphore(&fourthSemaphore);

	if (!inverse_transform(xr, xi, n, procNumber))
		goto cleanup;

	getVectorParcel(&start,&end, n, procNumber);
	for (i = start; i < end; i++) {
		dreal[i] = xr[i] / n;
		dimag[i] = xi[i] / n;
	}
	incrementSemaphore(&fifthSemaphore);
	acquireSemaphore(&fifthSemaphore);

	status = 1;

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