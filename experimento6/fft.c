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

volatile int *lock = (volatile int *) LOCK_ADDRESS;

volatile int *sin_op = (volatile int *)  SIN_ADDRESS;
volatile int *cos_op = (volatile int *)  COS_ADDRESS;

float sin_acc(float value){
        *sin_op = *((uint32_t*)&value);
        return *((float*)&(*sin_op));
}
float cos_acc(float value){
        *cos_op = *((uint32_t*)&value);
        return *((float*)&(*cos_op));
}

float cos_mod_two(float k, int n){
	return cos_acc(2 * M_PI * k / n);
}

float sin_mod_two(float k, int n){
	return sin_acc(2 * M_PI * k / n);
}

float cos_mod(float i, int n){
	return cos_acc( M_PI * i * i / n);
}

float sin_mod(float i, int n){
	return sin_acc( M_PI * i * i / n);
}

// Private function prototypes
static size_t reverse_bits(size_t x, unsigned int n);
static void *memdup(const void *src, size_t n);

#define N_MAX ((size_t)-1)


int transform(float real[], float imag[], size_t n) {
	if (n == 0)
		return 1;
	else if ((n & (n - 1)) == 0)  // Is power of 2
		return transform_radix2(real, imag, n);
	else  // More complicated algorithm for arbitrary sizes
		return transform_bluestein(real, imag, n);
}


int inverse_transform(float real[], float imag[], size_t n) {
	return transform(imag, real, n);
}


int transform_radix2(float real[], float imag[], size_t n) {
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


int transform_bluestein(float real[], float imag[], size_t n) {
	// Variables
	int status = 0;
	//float *cos_table, *sin_table;
	float *areal, *aimag;
	float *breal, *bimag;
	float *creal, *cimag;
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
	//cos_table = malloc(size_n);
	//sin_table = malloc(size_n);
	areal = calloc(m, sizeof(float));
	aimag = calloc(m, sizeof(float));
	breal = calloc(m, sizeof(float));
	bimag = calloc(m, sizeof(float));
	creal = malloc(size_m);
	cimag = malloc(size_m);
	if (areal == NULL || aimag == NULL
			|| breal == NULL || bimag == NULL
			|| creal == NULL || cimag == NULL)
		goto cleanup;
	
	// Trignometric tables
	/*for (i = 0; i < n; i++) {
		//float temp = M_PI * (size_t)((unsigned long long)i * i % ((unsigned long long)n * 2)) / n;
		// Less accurate version if long long is unavailable: float temp = M_PI * i * i / n;
		cos_table[i] = cos(temp);
		sin_table[i] = sin(temp);
	}*/
	
	// Temporary vectors and preprocessing
	for (i = 0; i < n; i++) {
		areal[i] =  real[i] * cos_mod(i,n) + imag[i] * sin_mod(i,n);
		aimag[i] = -real[i] * sin_mod(i,n) + imag[i] * cos_mod(i,n);
	}
	breal[0] = cos_mod(0,n);
	bimag[0] = sin_mod(0,n);
	for (i = 1; i < n; i++) {
		breal[i] = breal[m - i] = cos_mod(i,n);
		bimag[i] = bimag[m - i] = sin_mod(i,n);
	}
	
	// Convolution
	if (!convolve_complex(areal, aimag, breal, bimag, creal, cimag, m))
		goto cleanup;
	
	// Postprocessing
	for (i = 0; i < n; i++) {
		real[i] =  creal[i] * cos_mod(i,n) + cimag[i] * sin_mod(i,n);
		imag[i] = -creal[i] * sin_mod(i,n) + cimag[i] * cos_mod(i,n);
	}
	status = 1;
	
	// Deallocation
cleanup:
	free(cimag);
	free(creal);
	free(bimag);
	free(breal);
	free(aimag);
	free(areal);
	//free(sin_table);
	//free(cos_table);
	return status;
}


int convolve_real(const float x[], const float y[], float out[], size_t n) {
	float *ximag, *yimag, *zimag;
	int status = 0;
	ximag = calloc(n, sizeof(float));
	yimag = calloc(n, sizeof(float));
	zimag = calloc(n, sizeof(float));
	if (ximag == NULL || yimag == NULL || zimag == NULL)
		goto cleanup;
	
	status = convolve_complex(x, ximag, y, yimag, out, zimag, n);
cleanup:
	free(zimag);
	free(yimag);
	free(ximag);
	return status;
}


int convolve_complex(const float xreal[], const float ximag[], const float yreal[], const float yimag[], float outreal[], float outimag[], size_t n) {
	int status = 0;
	size_t size;
	size_t i;
	float *xr, *xi, *yr, *yi;
	if (N_MAX / sizeof(float) < n)
		return 0;
	size = n * sizeof(float);
	xr = memdup(xreal, size);
	xi = memdup(ximag, size);
	yr = memdup(yreal, size);
	yi = memdup(yimag, size);
	if (xr == NULL || xi == NULL || yr == NULL || yi == NULL)
		goto cleanup;
	
	if (!transform(xr, xi, n))
		goto cleanup;
	if (!transform(yr, yi, n))
		goto cleanup;
	for (i = 0; i < n; i++) {
		float temp = xr[i] * yr[i] - xi[i] * yi[i];
		xi[i] = xi[i] * yr[i] + xr[i] * yi[i];
		xr[i] = temp;
	}
	if (!inverse_transform(xr, xi, n))
		goto cleanup;
	for (i = 0; i < n; i++) {  // Scaling (because this FFT implementation omits it)
		outreal[i] = xr[i] / n;
		outimag[i] = xi[i] / n;
	}
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
