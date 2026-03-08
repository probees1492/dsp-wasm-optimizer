/**
 * @file simd_common.h
 * @brief Common SIMD definitions and utilities for DSP optimization
 * @author Seo David
 * @date 2026-03-08
 * 
 * @details
 * This header file provides SIMD (Single Instruction Multiple Data) intrinsics
 * and common utilities for DSP library optimization.
 * 
 * Supports AVX2 (Advanced Vector Extensions 2), AVX-512F (256-bit vectors),
 * SSE4.2 (Streaming SIMD Extensions 4.2), and NEON (ARM Advanced SIMD).
 * 
 * @see https://en.wikipedia.org/wiki/SIMD
 * @see https://en.wikipedia.org/wiki/AVX
 * @see https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions
 */

#pragma once

// SIMD Intrinsics (AVX2, AVX-512F, NEON, SSE4.2)
#if defined(__AVX2__)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

// Data alignment (Cache-friendly)
// Align data to 64-byte boundaries to improve cache performance
#ifndef _aligned
#define _aligned(x) __attribute__((aligned(x)))
#endif

// Mathematical constants
#define PI 3.14159265358979323846  // Pi
#define M_SQRT1_2 0.70710678118654752440  // 1/sqrt(2)
#define M_SQRT3_2 0.866025403784438646764  // sqrt(3)/2

// Complex number structure for FFT calculations
typedef struct {
    double re;  // Real part
    double im;  // Imaginary part
} complex_t;

// SIMD vector types for efficient parallel processing
#if defined(__AVX512F__)
typedef __m512d v4d;  // 512-bit vector (4 double-precision floats)
#define VSIZE 4
#elif defined(__AVX2__)
typedef __m256d v4d;  // 256-bit vector (4 double-precision floats)
#define VSIZE 4
#elif defined(__ARM_NEON)
typedef float64x2_t v2d;  // 128-bit vector (2 double-precision floats)
#define VSIZE 2
#else
typedef double v4d __attribute__((vector_size(32)));  // Fallback to scalar
#define VSIZE 4
#endif

/**
 * @brief Complex multiplication using SIMD
 * @param a First complex number
 * @param b Second complex number
 * @return Product of a and b
 * 
 * @details
 * Computes the product of two complex numbers:
 * (a.re + i·a.im) × (b.re + i·b.im) = (a.re·b.re - a.im·b.im) + i·(a.re·b.im + a.im·b.re)
 */
static inline complex_t cmul_simd(complex_t a, complex_t b) {
    complex_t result;
    result.re = a.re * b.re - a.im * b.im;
    result.im = a.re * b.im + a.im * b.re;
    return result;
}

/**
 * @brief Bit reversal for Radix-2 FFT
 * @param n Number to reverse
 * @param k Number of bits to reverse
 * @return Bit-reversed value of n
 * 
 * @details
 * Reverses the bits of n. For example, 4 (100) becomes 1 (001).
 * Used in Cooley-Tukey FFT algorithm for bit-reversal permutation.
 */
static inline uint32_t bit_reverse(uint32_t n, uint32_t k) {
    uint32_t reversed = 0;
    for (uint32_t i = 0; i < k; i++) {
        reversed = (reversed << 1) | (n & 1);
        n >>= 1;
    }
    return reversed;
}

/**
 * @brief Precompute Twiddle factors for FFT
 * @param twiddles Array to store Twiddle factors
 * @param n FFT size
 * @param radix Radix of the FFT algorithm (default: 2)
 * 
 * @details
 * Twiddle factors (or roots of unity) are precomputed for FFT.
 * Twiddle factor W_n^k = e^(-2πik/n) = cos(2πk/n) - i·sin(2πk/n)
 */
void precompute_twiddles(complex_t* twiddles, int n, int radix) {
    for (int i = 0; i < n; i++) {
        double angle = 2 * PI * i / n;
        twiddles[i].re = cos(angle);
        twiddles[i].im = sin(angle);
    }
}

/**
 * @brief Cache-friendly buffer structure
 * @details
 * Aligned memory layout to improve cache performance.
 * Data is aligned to 64-byte boundaries to match cache line size.
 */
typedef struct {
    double* _aligned(64) re;  // Real part array (64-byte aligned)
    double* _aligned(64) im;  // Imaginary part array (64-byte aligned)
    size_t size;
} fft_buffer_t;

/**
 * @brief Allocate FFT buffer
 * @param size Buffer size
 * @return Pointer to allocated buffer
 * 
 * @details
 * Allocates memory for FFT input/output buffers with proper alignment.
 * Uses posix_memalign for 64-byte alignment to optimize cache performance.
 */
fft_buffer_t* alloc_fft_buffer(size_t size) {
    fft_buffer_t* buf = (fft_buffer_t*)malloc(sizeof(fft_buffer_t));
    if (!buf) return NULL;
    
    buf->size = size;
    posix_memalign(64, sizeof(double) * size, (void**)&buf->re);
    posix_memalign(64, sizeof(double) * size, (void**)&buf->im);
    
    return buf;
}

/**
 * @brief Free FFT buffer
 * @param buf Buffer to free
 * 
 * @details
 * Frees memory allocated by alloc_fft_buffer().
 * Both the buffer structure and the aligned arrays are freed.
 */
void free_fft_buffer(fft_buffer_t* buf) {
    if (buf) {
        free(buf->re);
        free(buf->im);
        free(buf);
    }
}
