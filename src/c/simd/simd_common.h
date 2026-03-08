#pragma once

// SIMD 내장 함수 (AVX2, AVX-512, NEON, SSE4.2)
#if defined(__AVX2__)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

// 데이터 정렬 (Cache-friendly)
#ifndef _aligned
#define _aligned(x) __attribute__((aligned(x)))
#endif

// 복소수 상수
#define PI 3.14159265358979323846
#define M_SQRT1_2 0.70710678118654752440  // 1/sqrt(2)
#define M_SQRT3_2 0.866025403784438646764  // sqrt(3)/2

// 복소수 구조체
typedef struct {
    double re;
    double im;
} complex_t;

// SIMD 벡터 타입
#if defined(__AVX512F__)
typedef __m512d v4d;
#define VSIZE 4
#elif defined(__AVX2__)
typedef __m256d v4d;
#define VSIZE 4
#elif defined(__ARM_NEON)
typedef float64x2_t v2d;
#define VSIZE 2
#else
typedef double v4d __attribute__((vector_size(32)));
#define VSIZE 4
#endif

// 복소수 산술 SIMD 인라인 함수
static inline complex_t cmul_simd(complex_t a, complex_t b) {
    complex_t result;
    result.re = a.re * b.re - a.im * b.im;
    result.im = a.re * b.im + a.im * b.re;
    return result;
}

// 비트 리버스 (Radix-2 FFT 용)
static inline uint32_t bit_reverse(uint32_t n, uint32_t k) {
    uint32_t reversed = 0;
    for (uint32_t i = 0; i < k; i++) {
        reversed = (reversed << 1) | (n & 1);
        n >>= 1;
    }
    return reversed;
}

// SIMD-최적화 Twiddle 계수 사전 계산
void precompute_twiddles(complex_t* twiddles, int n, int radix) {
    for (int i = 0; i < n; i++) {
        double angle = 2 * PI * i / n;
        twiddles[i].re = cos(angle);
        twiddles[i].im = sin(angle);
    }
}

// Cache-friendly 데이터 구조
typedef struct {
    double* _aligned(64) re;  // 64바이트 정렬
    double* _aligned(64) im;
    size_t size;
} fft_buffer_t;

// FFT 버퍼 할당
fft_buffer_t* alloc_fft_buffer(size_t size) {
    fft_buffer_t* buf = (fft_buffer_t*)malloc(sizeof(fft_buffer_t));
    if (!buf) return NULL;
    
    buf->size = size;
    posix_memalign(64, sizeof(double) * size, (void**)&buf->re);
    posix_memalign(64, sizeof(double) * size, (void**)&buf->im);
    
    return buf;
}

// FFT 버퍼 해제
void free_fft_buffer(fft_buffer_t* buf) {
    if (buf) {
        free(buf->re);
        free(buf->im);
        free(buf);
    }
}
