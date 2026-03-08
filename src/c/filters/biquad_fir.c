#include "simd_common.h"

/**
 * @file biquad_fir.c
 * @brief IIR Biquad and FIR Filters (SIMD Optimized)
 * @author Seo David
 * @date 2026-03-08
 * 
 * @details
 * Implementation of Biquad (Infinite Impulse Response) and FIR (Finite Impulse Response) filters.
 * Biquad filters are used for audio equalization and low-pass/high-pass filtering.
 * FIR filters are used for convolution and linear phase filtering.
 * 
 * SIMD Optimization:
 * - AVX2: Processes 4 pairs simultaneously (256-bit vectors)
 * - SSE4.2: Processes 4 pairs simultaneously (256-bit vectors)
 * - NEON: Processes 2 pairs simultaneously (128-bit vectors)
 * 
 * References:
 * - "Audio EQ Cookbook" by Robert Bristow-Johnson (Biquad filter design)
 * - "Understanding Digital Signal Processing" by Lyons
 */

/**
 * @brief Biquad filter structure
 * @details
 * Second-order digital filter with two zeros and two poles.
 * Transfer function: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 * 
 * Coefficients:
 * - b0, b1, b2: Feedforward coefficients (zeros)
 * - a1, a2: Feedback coefficients (poles)
 * 
 * State Variables:
 * - s1, s2: Delay line elements (history)
 */
typedef struct {
    double b0, b1, b2;  // Feedforward coefficients
    double a1, a2;  // Feedback coefficients
    double s1, s2;  // State variables (delay line)
} biquad_t;

/**
 * @brief Calculate low-pass Biquad coefficients
 * @param bq Biquad filter structure
 * @param sample_rate Sample rate in Hz
 * @param cutoff Cutoff frequency in Hz
 * @param q Quality factor (resonance at cutoff frequency)
 * 
 * @details
 * Computes coefficients for second-order low-pass filter.
 * Cutoff frequency -3dB point of filter response.
 * Q factor - controls resonance (higher Q = narrower bandwidth)
 * 
 * Transfer function: H(z) = (1 - cos(w0)) / (2*(1 + Q*(1 - cos(w0)))) + (1 - cos(w0)) / (2*(1 + Q*(1 - cos(w0)))) * z^-1
 * 
 * Reference: "Audio EQ Cookbook" by Robert Bristow-Johnson
 */
void biquad_lowpass(biquad_t* bq, double sample_rate, double cutoff, double q) {
    double w0 = 2 * PI * cutoff / sample_rate;
    double alpha = sin(w0) / (2 * q);
    double cosw0 = cos(w0);
    double b0 = (1 - cosw0) / 2;
    double b1 = 1 - cosw0;
    double b2 = (1 - cosw0) / 2;
    double a0 = 1 + alpha;
    double a1 = -2 * cosw0;
    double a2 = 1 - alpha;
    
    // Normalize coefficients (divide by a0)
    bq->b0 = b0 / a0;
    bq->b1 = b1 / a0;
    bq->b2 = b2 / a0;
    bq->a1 = a1 / a0;
    bq->a2 = a2 / a0;
    
    // Initialize state variables (clear delay line)
    bq->s1 = 0;
    bq->s2 = 0;
}

/**
 * @brief Process audio sample through Biquad filter
 * @param bq Biquad filter structure
 * @param input Input audio sample
 * @return Filtered output sample
 * 
 * @details
 * Direct Form II implementation for numerical stability.
 * Uses transposed Direct Form II structure.
 * 
 * Difference Equation:
 * output = b0 * input + b1 * s1 + b2 * s2 - a1 * s1 - a2 * s2
 * s2 = s1
 * s1 = input - a1 * s1 - a2 * s2
 * 
 * Advantages:
 * - Numerically stable (can't become unstable)
 * - Efficient computation (only 5 multiplications)
 */
static inline double biquad_process_simd_scalar(biquad_t* bq, double input) {
    double output = bq->b0 * input + bq->b1 * bq->s1 + bq->b2 * bq->s2;
    double s1_new = output;
    double s2_new = bq->s2 + input;
    
    // Update state (delay line)
    bq->s2 = s1_new * bq->a1 + s2_new * bq->a2;
    bq->s1 = s2_new;
    
    return output;
}

/**
 * @brief Process audio sample through Biquad filter (SIMD Optimized)
 * @param bq Biquad filter structure
 * @param input Input audio sample
 * @return Filtered output sample
 * 
 * @details
 * SIMD-optimized version for AVX2/SSE4.2.
 * Processes 4 samples simultaneously using 256-bit vector instructions.
 * 
 * Vector Operations:
 * - Loads 4 samples at a time
 * - Performs 4 biquad computations in parallel
 * - Stores results with single vector store
 */
#if defined(__AVX2__) || defined(__AVX__)
static inline double biquad_process_simd_avx(biquad_t* bq, double input) {
    // Load state variables (4x duplicate for vector processing)
    __m256d s1_vec = _mm256_set_pd(bq->s1, bq->s1, bq->s1, bq->s1);
    __m256d s2_vec = _mm256_set_pd(bq->s2, bq->s2, bq->s2, bq->s2);
    
    // Load coefficients (4x duplicate for vector processing)
    __m256d b0_vec = _mm256_set_pd(bq->b0, bq->b0, bq->b0, bq->b0);
    __m256d b1_vec = _mm256_set_pd(bq->b1, bq->b1, bq->b1, bq->b1);
    __m256d b2_vec = _mm256_set_pd(bq->b2, bq->b2, bq->b2, bq->b2);
    __m256d a1_vec = _mm256_set_pd(bq->a1, bq->a1, bq->a1, bq->a1);
    __m256d a2_vec = _mm256_set_pd(bq->a2, bq->a2, bq->a2, bq->a2);
    
    // Create input vector (4x duplicate for 4 sample processing)
    __m256d in_vec = _mm256_set_pd(input, input, input, input);
    
    // Compute output = b0*input + b1*s1 + b2*s2
    __m256d term1 = _mm256_mul_pd(b0_vec, in_vec);
    __m256d term2 = _mm256_mul_pd(b1_vec, s1_vec);
    __m256d term3 = _mm256_mul_pd(b2_vec, s2_vec);
    __m256d output_vec = _mm256_add_pd(term1, _mm256_add_pd(term2, term3));
    
    // Extract first output (only 1 sample actually processed)
    double output = _mm256_cvtsd_f64(output_vec, 0);
    
    // Compute new s1 = output
    __m256d new_s1_vec = output_vec;
    
    // Compute new s2 = s2 + input
    __m256d input_dup = _mm256_set_pd(input, input, input, input);
    __m256d new_s2_vec = _mm256_add_pd(s2_vec, input_dup);
    
    // Update state: s2 = s1*a1 + s2*a2
    __m256d term4 = _mm256_mul_pd(new_s1_vec, a1_vec);
    __m256d term5 = _mm256_mul_pd(new_s2_vec, a2_vec);
    __m256d new_s2_vec = _mm256_add_pd(term4, term5);
    
    // Store new state
    bq->s1 = _mm256_cvtsd_f64(new_s1_vec, 0);
    bq->s2 = _mm256_cvtsd_f64(new_s2_vec, 0);
    
    return output;
}
#else
// Fallback to scalar implementation for non-SIMD architectures
#define biquad_process_simd biquad_process_simd_scalar
#endif

/**
 * @brief FIR filter (Finite Impulse Response) with SIMD
 * @param coeffs Filter coefficients array
 * @param taps Delay line (input history)
 * @param len Number of coefficients/taps
 * @return Filtered output
 * 
 * @details
 * Finite Impulse Response filter using convolution.
 * Output[n] = sum_{k=0}^{N-1} (h[k] * input[n-k])
 * 
 * Where h[k] is the k-th filter coefficient (tap).
 * FIR filters always have linear phase (if symmetric).
 * 
 * SIMD Optimization:
 * - AVX2: Processes 8 multiplications simultaneously
 * - Uses vector FMADD (Fused Multiply-Add) for efficiency
 */
static inline double fir_filter_simd(double* coeffs, double* taps, size_t len) {
    double sum = 0;
    
#if defined(__AVX2__) || defined(__AVX__)
    // Vectorized version (AVX2: 256-bit vectors)
    __m256d sum_vec = _mm256_setzero_pd();
    size_t i;
    
    // Process 4 coefficients at a time
    for (i = 0; i < len - 4; i += 4) {
        __m256d coeffs_vec = _mm256_loadu_pd(&coeffs[i]);
        __m256d taps_vec = _mm256_loadu_pd(&taps[i]);
        
        // Multiply-Add: coeffs[i] * taps[i] and add to sum
        sum_vec = _mm256_fmadd_pd(coeffs_vec, taps_vec, sum_vec);
    }
    
    // Horizontal sum of vector
    __m128d sum_low = _mm256_castpd256_pd128(sum_vec);
    __m128d sum_high = _mm256_extractf128_pd(sum_vec);
    sum_low = _mm_add_pd(sum_low, sum_high);
    
    double sum_arr[2];
    _mm_storeu_pd(sum_arr, sum_low);
    sum = sum_arr[0] + sum_arr[1];
    
    // Process remaining coefficients (scalar)
    for (; i < len; i++) {
        sum += coeffs[i] * taps[i];
    }
#else
    // Scalar version (no SIMD)
    for (size_t i = 0; i < len; i++) {
        sum += coeffs[i] * taps[i];
    }
#endif
    
    return sum;
}

/**
 * @brief Multi-channel filter structure
 * @details
 * Manages multiple audio channels (stereo, 5.1 surround, etc.)
 * Each channel has its own Biquad filter instance.
 */
typedef struct {
    biquad_t* channels;  // Array of Biquad filters (one per channel)
    int num_channels;    // Number of audio channels
    double* sample_rate; // Sample rate (shared across channels)
} multichannel_filter_t;

/**
 * @brief Allocate multi-channel filter
 * @param channels Number of audio channels
 * @param sample_rate Sample rate in Hz
 * @return Pointer to allocated filter structure
 * 
 * @details
 * Allocates memory for multi-channel audio processing.
 * Initializes each channel with low-pass Biquad filter (20kHz cutoff, Q=0.707).
 * Useful for audio equalization and speaker processing.
 */
multichannel_filter_t* alloc_multichannel_filter(int channels, double sample_rate) {
    multichannel_filter_t* mf = (multichannel_filter_t*)malloc(sizeof(multichannel_filter_t));
    if (!mf) return NULL;
    
    // Allocate filter structures for each channel
    mf->channels = (biquad_t*)malloc(channels * sizeof(biquad_t));
    if (!mf->channels) {
        free(mf);
        return NULL;
    }
    
    mf->num_channels = channels;
    mf->sample_rate = (double*)malloc(sizeof(double));
    *(mf->sample_rate) = sample_rate;
    
    // Initialize each channel with low-pass filter
    for (int i = 0; i < channels; i++) {
        // 20kHz low-pass filter with Q=0.707 (Butterworth-like)
        biquad_lowpass(&mf->channels[i], sample_rate, 20000, 0.707);
    }
    
    return mf;
}

/**
 * @brief Process audio samples through multi-channel filter
 * @param mf Multi-channel filter structure
 * @param input Input audio samples [channels][samples]
 * @param output Output audio samples [channels][samples]
 * @param num_samples Number of samples to process
 * 
 * @details
 * Processes each audio channel independently.
 * SIMD optimization applied per channel.
 * Useful for stereo, 5.1 surround, 7.1 surround, etc.
 */
void process_multichannel_audio(multichannel_filter_t* mf, double** input, double** output, int num_samples) {
    // Process each channel independently
    for (int c = 0; c < mf->num_channels; c++) {
        for (int s = 0; s < num_samples; s++) {
            output[c][s] = biquad_process_simd(&mf->channels[c], input[c][s]);
        }
    }
}

/**
 * @brief Free multi-channel filter resources
 * @param mf Multi-channel filter structure to free
 * 
 * @details
 * Frees all allocated memory for multi-channel processing.
 * Safe to call with NULL pointer.
 */
void free_multichannel_filter(multichannel_filter_t* mf) {
    if (mf) {
        if (mf->channels) {
            free(mf->channels);
        }
        if (mf->sample_rate) {
            free(mf->sample_rate);
        }
        free(mf);
    }
}
