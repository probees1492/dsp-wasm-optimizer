#include "simd_common.h"

/**
 * @file fft_radix2.c
 * @brief Radix-2 Decimation-in-Time FFT (SIMD Optimized)
 * @author Seo David
 * @date 2026-03-08
 * 
 * @details
 * Fast Fourier Transform (FFT) using Radix-2 Cooley-Tukey algorithm.
 * Optimized with SIMD instructions (AVX2, AVX-512, NEON, SSE4.2).
 * 
 * Time Complexity: O(n log n)
 * Space Complexity: O(n)
 * 
 * @see https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm
 * @see https://en.wikipedia.org/wiki/Radix
 */

// Bit reversal for Radix-2 FFT
/**
 * @brief Reverse the bits of an integer
 * @param n Integer to reverse
 * @param bits Number of bits to reverse
 * @return Bit-reversed value of n
 * 
 * @details
 * Reverses the bits of n. For example, 4 (100) becomes 1 (001).
 * Used in Cooley-Tukey FFT for bit-reversal permutation.
 */
static uint32_t bit_reverse(uint32_t n, uint32_t bits) {
    uint32_t reversed = 0;
    for (uint32_t i = 0; i < bits; i++) {
        reversed = (reversed << 1) | (n & 1);
        n >>= 1;
    }
    return reversed;
}

// Butterfly operation (SIMD Optimized version)
/**
 * @brief Perform butterfly operation using SIMD
 * @param re Real part array (32-byte aligned)
 * @param im Imaginary part array (32-byte aligned)
 * @param start Starting index
 * @param end Ending index
 * @param twiddles Pre-computed Twiddle factors
 * @param twiddle_idx Current Twiddle index
 * 
 * @details
 * Butterfly operation in FFT combines two inputs into two outputs.
 * SIMD version processes 4 pairs simultaneously using AVX2/SSE4.2 instructions.
 * For each pair:
 * - Upper output = z + w*z'
 * - Lower output = z - w*z'
 * where w is the Twiddle factor (root of unity).
 */
#if defined(__AVX2__) || defined(__AVX__)
static inline void butterfly_simd(
    double* _aligned(32) re, double* _aligned(32) im,
    size_t start, size_t end,
    const complex_t* twiddles, size_t twiddle_idx
) {
    // Load Twiddle factors (4 pairs at a time)
    __m256d tw = _mm256_set_pd(twiddles[twiddle_idx].re, twiddles[twiddle_idx].im,
                              twiddles[twiddle_idx + 1].re, twiddles[twiddle_idx + 1].im,
                              twiddles[twiddle_idx].re, twiddles[twiddle_idx].im,
                              twiddles[twiddle_idx + 1].re, twiddles[twiddle_idx + 1].im);
    
    // Process 4 pairs of complex numbers
    for (size_t i = start; i < end; i += 4) {
        // Load 4 complex numbers (8 double-precision floats)
        __m256d vr = _mm256_load_pd(&re[i]);
        __m256d vi = _mm256_load_pd(&im[i]);
        
        // Compute w * z'
        // (w.re * z.re - w.im * z.im, w.re * z.im + w.im * z.re)
        __m256d w_zr = _mm256_mul_pd(tw, vr);
        __m256d w_zi = _mm256_mul_pd(tw, vi);
        
        // Compute z + w*z'
        // (z.re + w_zr.re, z.im + w_zi.im)
        __m256d ar = _mm256_add_pd(vr, w_zr);
        __m256d ai = _mm256_add_pd(vi, w_zi);
        
        // Store results
        _mm256_store_pd(&re[i], ar);
        _mm256_store_pd(&im[i], ai);
        
        // Update Twiddle factor for next pair
        tw = _mm256_set_pd(twiddles[twiddle_idx + 2].re, twiddles[twiddle_idx + 2].im,
                          twiddles[twiddle_idx + 3].re, twiddles[twiddle_idx + 3].im,
                          twiddles[twiddle_idx + 2].re, twiddles[twiddle_idx + 2].im,
                          twiddles[twiddle_idx + 3].re, twiddles[twiddle_idx + 3].im);
        
        // Load next 4 complex numbers
        __m256d vr2 = _mm256_load_pd(&re[i + 2]);
        __m256d vi2 = _mm256_load_pd(&im[i + 2]);
        
        // Compute w * z' for next 2 pairs
        __m256d w_zr2 = _mm256_mul_pd(tw, vr2);
        __m256d w_zi2 = _mm256_mul_pd(tw, vi2);
        
        // Compute z + w*z'
        __m256d ar2 = _mm256_add_pd(vr2, w_zr2);
        __m256d ai2 = _mm256_add_pd(vi2, w_zi2);
        
        // Store results
        _mm256_store_pd(&re[i + 2], ar2);
        _mm256_store_pd(&im[i + 2], ai2);
        
        // Advance Twiddle index by 4
        twiddle_idx += 4;
    }
}
#else
// Fallback to scalar implementation for non-SIMD architectures
static inline void butterfly_simd(
    double* _aligned(32) re, double* _aligned(32) im,
    size_t start, size_t end,
    const complex_t* twiddles, size_t twiddle_idx
) {
    // Scalar version (processes 1 pair at a time)
    for (size_t i = start; i < end; i += 2) {
        double w_re = twiddles[twiddle_idx].re;
        double w_im = twiddles[twiddle_idx].im;
        
        double z_re = re[i];
        double z_im = im[i];
        
        // Compute w * z'
        double w_zr_re = w_re * z_re - w_im * z_im;
        double w_zr_im = w_re * z_im + w_im * z_re;
        
        // Compute z + w*z'
        re[i] = z_re + w_zr_re;
        im[i] = z_im + w_zr_im;
        
        // Increment Twiddle index
        twiddle_idx++;
    }
}
#endif

/**
 * @brief Cooley-Tukey FFT (SIMD Optimized)
 * @param re Real part array
 * @param im Imaginary part array
 * @param n FFT size (must be power of 2)
 * @param twiddles Pre-computed Twiddle factors
 * 
 * @details
 * Cooley-Tukey FFT algorithm with Radix-2 decimation-in-time.
 * 1. Bit Reversal: Reorder input data
 * 2. Recursively apply FFT to N1 and N2 (where N = N1 * N2)
 * 3. Combine results using Twiddle factors
 * 
 * SIMD Optimized:
 * - AVX2 processes 4 pairs simultaneously
 * - Cache-friendly data alignment (32-byte)
 * - Pre-computed Twiddle factors
 */
void fft_radix2_simd(double* re, double* im, int n, const complex_t* twiddles) {
    // Step 1: Calculate number of bits needed to represent n
    int bits = 0;
    int temp = n - 1;
    while (temp > 0) {
        bits++;
        temp >>= 1;
    }
    
    // Step 2: Bit Reversal (Permute input data)
    // For example, 0, 1, 2, 3, 4, 5, 6, 7 becomes 0, 4, 2, 6, 1, 5, 3, 7
    for (int i = 0; i < n; i++) {
        int j = bit_reverse(i, bits);
        if (i < j) {
            // Swap real and imaginary parts
            double temp_re = re[i];
            double temp_im = im[i];
            re[i] = re[j];
            im[i] = im[j];
            re[j] = temp_re;
            im[j] = temp_im;
        }
    }
    
    // Step 3: Cooley-Tukey FFT (Divide-and-Conquer)
    // For each size = 2, 4, 8, 16, ..., n
    // Perform butterfly operations with Twiddle factors
    for (int size = 2; size <= n; size <<= 1) {
        int halfsize = size >> 1;  // size / 2
        
        // For each stage...
        for (int i = 0; i < n; i += size) {
            // Perform butterfly operations between pairs i + j and i + j + halfsize
            // SIMD version processes 4 pairs at a time (when size >= 8)
            // Scalar version processes 1 pair at a time (fallback)
            #if defined(__AVX2__) || defined(__AVX__)
            if (size >= 8) {
                // SIMD optimized version (processes 4 pairs)
                butterfly_simd(re, im, i, i + halfsize, twiddles, i);
            } else {
                // Scalar version (processes 1 pair)
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
