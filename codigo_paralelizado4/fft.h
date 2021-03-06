/*
 * Free FFT and convolution (C)
 *
 * Copyright (c) 2016 Project Nayuki
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

/*
 * Aceleração da função de seno para ponto flutuante
 */
float sin_acc(float value);

/*
 * Aceleração da função de coseno para ponto flutuante
 */
float cos_acc(float value);

/*
 * Aceleração da função de soma para ponto flutuante
 */
float sum_acc(float value_a, float value_b);

/*
 * Aceleração da função de subtração para ponto flutuante
 */
float sub_acc(float value_a, float value_b);

/*
 * Aceleração da função de multiplicação para ponto flutuante
 */
float mul_acc(float value_a, float value_b);

/*
 * Aceleração da função de divisão para ponto flutuante
 */
float div_acc(float value_a, float value_b);

/*
 * Computes the discrete Fourier transform (DFT) of the given complex vector, storing the result back into the vector.
 * The vector can have any length. This is a wrapper function. Returns 1 (true) if successful, 0 (false) otherwise (out of memory).
 */
int transform(float real[], float imag[], size_t n, int procNumber);

/*
 * Computes the inverse discrete Fourier transform (IDFT) of the given complex vector, storing the result back into the vector.
 * The vector can have any length. This is a wrapper function. This transform does not perform scaling, so the inverse is not a true inverse.
 * Returns 1 (true) if successful, 0 (false) otherwise (out of memory).
 */
int inverse_transform(float real[], float imag[], size_t n, int procNumber);

/*
 * Computes the discrete Fourier transform (DFT) of the given complex vector, storing the result back into the vector.
 * The vector's length must be a power of 2. Uses the Cooley-Tukey decimation-in-time radix-2 algorithm.
 * Returns 1 (true) if successful, 0 (false) otherwise (n is not a power of 2, or out of memory).
 */
int transform_radix2(float real[], float imag[], size_t n, int procNumber);

/*
 * Computes the discrete Fourier transform (DFT) of the given complex vector, storing the result back into the vector.
 * The vector can have any length. This requires the convolution function, which in turn requires the radix-2 FFT function.
 * Uses Bluestein's chirp z-transform algorithm. Returns 1 (true) if successful, 0 (false) otherwise (out of memory).
 */
int transform_bluestein(float real[], float imag[], size_t n, int procNumber);


/*
 * Computes the circular convolution of the given complex vectors. Each vector's length must be the same.
 * Returns 1 (true) if successful, 0 (false) otherwise (out of memory).
 */
int convolve_complex(float* breal, float* bimag, size_t n, int procNumber);
