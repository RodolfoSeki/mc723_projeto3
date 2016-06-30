/*
 * FFT and convolution test (C)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fft.h"
#include "input.h"
#define NUMBER_OF_CORES
#define LOCK_ADDRESS 0x6400000

// Private function prototypes
static void test_fft(int n, int procNumber);
static void test_convolution(int n);
static void naive_dft(const float *inreal, const float *inimag, float *outreal, float *outimag, int inverse, int n);
static float log10_rms_err(const float *xreal, const float *ximag, const float *yreal, const float *yimag, int n);
static float *random_reals(int n);
static float *random_imag(int n);
static void *memdup(const void *src, size_t n);
static float max_log_error = -INFINITY;

volatile int procCounter = 0;

/* Main and test functions */

int main(int argc, char **argv) {
	int i;
	int prev;
	srand(10);

	int procNumber;
	AcquireLock();
	procNumber = procCounter;
	procCounter++;
	ReleaseLock();

	// Test small size FFTs
	for (i = 0; i < 64; i++)
		test_fft(i, procNumber);

	printf("\n");

	return 0;
}


static void test_fft(int n, int procNumber) {
	float *inputreal, *inputimag;
	float *refoutreal, *refoutimag;
	float *actualoutreal, *actualoutimag;
	int i;

	inputreal = random_reals(n);
	inputimag = random_imag(n);

	refoutreal = malloc(n * sizeof(float));
	refoutimag = malloc(n * sizeof(float));
	naive_dft(inputreal, inputimag, refoutreal, refoutimag, 0, n);

	actualoutreal = memdup(inputreal, n * sizeof(float));
	actualoutimag = memdup(inputimag, n * sizeof(float));
	transform(actualoutreal, actualoutimag, n, procNumber);

	if (procNumber == 0){
		printf("fftsize=%4d  logerr=%5.1f\n", n, log10_rms_err(refoutreal, refoutimag, actualoutreal, actualoutimag, n));

		/* Imprime o vetor de saida da transformada de Fourier*/
		printf(" Parte real | Parte Imaginária\n");
		for(i = 0; i < n; i++){
			printf("%10.4f  | %10.4f\n", actualoutreal[i], actualoutimag[i]);
		}
	}

	free(refoutreal);
	free(refoutimag);
	free(actualoutreal);
	free(actualoutimag);
}

/* Naive reference computation functions */

static void naive_dft(const float *inreal, const float *inimag, float *outreal, float *outimag, int inverse, int n) {
	float coef = (inverse ? 2 : -2) * M_PI;
	int k;
	for (k = 0; k < n; k++) {  // For each output element
		float sumreal = 0;
		float sumimag = 0;
		int t;
		for (t = 0; t < n; t++) {  // For each input element
			float angle = coef * ((long long)t * k % n) / n;
			sumreal += inreal[t]*cos(angle) - inimag[t]*sin(angle);
			sumimag += inreal[t]*sin(angle) + inimag[t]*cos(angle);
		}
		outreal[k] = sumreal;
		outimag[k] = sumimag;
	}
}

/* Utility functions */

static float log10_rms_err(const float *xreal, const float *ximag, const float *yreal, const float *yimag, int n) {
	float err = 0;
	int i;
	for (i = 0; i < n; i++)
		err += (xreal[i] - yreal[i]) * (xreal[i] - yreal[i]) + (ximag[i] - yimag[i]) * (ximag[i] - yimag[i]);

	err /= n > 0 ? n : 1;
	err = sqrt(err);  // Now this is a root mean square (RMS) error
	err = err > 0 ? log10(err) : -99.0;
	if (err > max_log_error)
		max_log_error = err;
	return err;
}


static float *random_reals(int n) {
	return input_real[n];
}


static float *random_imag(int n) {
	return input_imag[n];
}


static void *memdup(const void *src, size_t n) {
	void *dest = malloc(n);
	if (dest != NULL)
		memcpy(dest, src, n);
	return dest;
}
