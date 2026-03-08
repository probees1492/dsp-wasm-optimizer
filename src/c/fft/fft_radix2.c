#include "simd_common.h"

// Radix-2 Decimation-in-Time FFT (SIMD 최적화)
// Reference: "Simple FFT" by J. S. Smith

// 비트 리버스
static uint32_t bit_reverse(uint32_t n, uint32_t bits) {
    uint32_t reversed = 0;
    for (uint32_t i = 0; i < bits; i++) {
        reversed = (reversed << 1) | (n & 1);
        n >>= 1;
    }
    return reversed;
}

// Butterfly 연산 (SIMD 최적화 버젼)
#if defined(__AVX2__) || defined(__AVX__)
static inline void butterfly_simd(
    double* _aligned(32) re, double* _aligned(32) im,
    size_t start, size_t end,
    const complex_t* twiddles, size_t twiddle_idx
) {
    __m256d tw = _mm256_set_pd(twiddles[twiddle_idx].re, twiddles[twiddle_idx].im,
                              twiddles[twiddle_idx].re, twiddles[twiddle_idx].im);
    
    for (size_t i = start; i < end; i += 4) {
        __m256d vr = _mm256_load_pd(&re[i]);
        __m256d vi = _mm256_load_pd(&im[i]);
        
        // w * z'
        __m256d w_zr = _mm256_mul_pd(tw, vr);
        __m256d w_zi = _mm256_mul_pd(tw, vi);
        
        // z + w*z'
        __m256d ar = _mm256_add_pd(vr, w_zr);
        __m256d ai = _mm256_add_pd(vi, w_zi);
        
        _mm256_store_pd(&re[i], ar);
        _mm256_store_pd(&im[i], ai);
        
        tw = _mm256_set_pd(twiddles[twiddle_idx + 1].re, twiddles[twiddle_idx + 1].im,
                          twiddles[twiddle_idx + 1].re, twiddles[twiddle_idx + 1].im);
        
        __m256d vr2 = _mm256_load_pd(&re[i + 2]);
        __m256d vi2 = _mm256_load_pd(&im[i + 2]);
        
        __m256d w_zr2 = _mm256_mul_pd(tw, vr2);
        __m256d w_zi2 = _mm256_mul_pd(tw, vi2);
        
        __m256d ar2 = _mm256_add_pd(vr2, w_zr2);
        __m256d ai2 = _mm256_add_pd(vi2, w_zi2);
        
        _mm256_store_pd(&re[i + 2], ar2);
        _mm256_store_pd(&im[i + 2], ai2);
        
        twiddle_idx += 2;
    }
}
#else
static inline void butterfly_simd(
    double* _aligned(32) re, double* _aligned(32) im,
    size_t start, size_t end,
    const complex_t* twiddles, size_t twiddle_idx
) {
    for (size_t i = start; i < end; i += 2) {
        double w_re = twiddles[twiddle_idx].re;
        double w_im = twiddles[twiddle_idx].im;
        
        double z_re = re[i];
        double z_im = im[i];
        
        double w_zr_re = w_re * z_re - w_im * z_im;
        double w_zr_im = w_re * z_im + w_im * z_re;
        
        re[i] = z_re + w_zr_re;
        im[i] = z_im + w_zr_im;
        
        twiddle_idx++;
    }
}
#endif

// Cooley-Tukey FFT (SIMD 최적화)
void fft_radix2_simd(double* re, double* im, int n, const complex_t* twiddles) {
    int bits = 0;
    int temp = n - 1;
    while (temp > 0) {
        bits++;
        temp >>= 1;
    }
    
    // Bit Reversal
    for (int i = 0; i < n; i++) {
        int j = bit_reverse(i, bits);
        if (i < j) {
            double temp_re = re[i];
            double temp_im = im[i];
            re[i] = re[j];
            im[i] = im[j];
            re[j] = temp_re;
            im[j] = temp_im;
        }
    }
    
    // Cooley-Tukey FFT
    for (int size = 2; size <= n; size <<= 1) {
        int halfsize = size >> 1;
        
        // SIMD 버터플라이 처리
        #if defined(__AVX2__) || defined(__AVX__)
        //if (size >= 8) {
            // SIMD 최적화 버젼
        //    for (int i = 0; i < n; i += size) {
        //        butterfly_simd(re, im, i, i + halfsize, twiddles, i);
        //    }
        //} else {
            // 스칼라 버젼
            for (int i = 0; i < n; i += size) {
                for (int j = 0; j < halfsize; j++) {
                    int k = i + j;
                    int l = k + halfsize;
                    
                    double w_re = twiddles[j].re;
                    double w_im = twiddles[j].im;
                    
                    double z_re = re[k];
                    double z_im = im[k];
                    
                    double w_zr_re = w_re * z_re - w_im * z_im;
                    double w_zr_im = w_re * z_im + w_im * z_re;
                    
                    re[k] = z_re + w_zr_re;
                    im[k] = z_im + w_zr_im;
                }
            }
        }
    }
}
