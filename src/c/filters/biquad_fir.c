#include "simd_common.h"

// IIR Biquad 필터 (Low-pass)
// 참고: "Audio EQ Cookbook" by Robert Bristow-Johnson

typedef struct {
    double b0, b1, b2;
    double a1, a2;
    double s1, s2;
} biquad_t;

// Biquad 계수 계산 (Low-pass)
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
    
    // 계수 저장
    bq->b0 = b0 / a0;
    bq->b1 = b1 / a0;
    bq->b2 = b2 / a0;
    bq->a1 = a1 / a0;
    bq->a2 = a2 / a0;
    
    // 상태 초기화
    bq->s1 = 0;
    bq->s2 = 0;
}

// IIR 필터 처리 (SIMD 최적화)
#if defined(__AVX2__) || defined(__AVX__)
static inline double biquad_process_simd_avx(
    biquad_t* bq, double input
) {
    // SIMD 버전
    __m256d in_vec = _mm256_set_pd(input, input, input, input);
    __m256d s1_vec = _mm256_set_pd(bq->s1, bq->s1, bq->s1, bq->s1);
    __m256d s2_vec = _mm256_set_pd(bq->s2, bq->s2, bq->s2, bq->s2);
    
    __m256d b0_vec = _mm256_set_pd(bq->b0, bq->b0, bq->b0, bq->b0);
    __m256d b1_vec = _mm256_set_pd(bq->b1, bq->b1, bq->b1, bq->b1);
    __m256d b2_vec = _mm256_set_pd(bq->b2, bq->b2, bq->b2, bq->b2);
    __m256d a1_vec = _mm256_set_pd(bq->a1, bq->a1, bq->a1, bq->a1);
    __m256d a2_vec = _mm256_set_pd(bq->a2, bq->a2, bq->a2, bq->a2);
    
    // s1 = input * b0 + s1 * b1 + s2 * b2
    __m256d out1 = _mm256_fmadd_pd(b0_vec, in_vec, 
                                   _mm256_fmadd_pd(b1_vec, s1_vec, 
                                                     _mm256_mul_pd(b2_vec, s2_vec)));
    bq->s1 = _mm256_cvtsd_f64(out1, 1);
    
    // s2 = s1 * a1 + s2 * a2
    __m256d out2 = _mm256_fmadd_pd(a1_vec, out1, 
                                   _mm256_mul_pd(a2_vec, s2_vec));
    bq->s2 = _mm256_cvtsd_f64(out2, 1);
    
    // output = s1 + s2
    double output = bq->s1 + bq->s2;
    return output;
}
#else
static inline double biquad_process_simd_scalar(biquad_t* bq, double input) {
    double output = bq->b0 * input + bq->b1 * bq->s1 + bq->b2 * bq->s2;
    double s1_new = output;
    double s2_new = bq->s2 + input;
    
    bq->s2 = s1_new * bq->a1 + s2_new * bq->a2;
    bq->s1 = s2_new;
    
    return output;
}
#define biquad_process_simd biquad_process_simd_scalar
#endif

// FIR 필터 (SIMD 최적화)
static inline double fir_filter_simd(double* coeffs, double* taps, size_t len) {
    double sum = 0;
    
#if defined(__AVX2__) || defined(__AVX__)
    __m256d sum_vec = _mm256_setzero_pd();
    size_t i;
    for (i = 0; i < len - 4; i += 4) {
        __m256d coeffs_vec = _mm256_loadu_pd(&coeffs[i]);
        __m256d taps_vec = _mm256_loadu_pd(&taps[i]);
        sum_vec = _mm256_fmadd_pd(coeffs_vec, taps_vec, sum_vec);
    }
    
    // 남은 요소들 처리
    __m128d sum_low = _mm256_castpd256_pd128(sum_vec);
    double sum_arr[4];
    _mm_storeu_pd(sum_arr, sum_low);
    for (; i < len; i++) {
        sum_arr[i % 4] += coeffs[i] * taps[i];
    }
    sum = sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];
#else
    for (i = 0; i < len; i++) {
        sum += coeffs[i] * taps[i];
    }
#endif
    
    return sum;
}

// 다채널 멀티 필터
typedef struct {
    biquad_t* channels;
    int num_channels;
    double* sample_rate;
} multichannel_filter_t;

multichannel_filter_t* alloc_multichannel_filter(int channels, double sample_rate) {
    multichannel_filter_t* mf = (multichannel_filter_t*)malloc(sizeof(multichannel_filter_t));
    if (!mf) return NULL;
    
    mf->channels = (biquad_t*)malloc(channels * sizeof(biquad_t));
    if (!mf->channels) {
        free(mf);
        return NULL;
    }
    
    mf->num_channels = channels;
    mf->sample_rate = (double*)malloc(sizeof(double));
    *(mf->sample_rate) = sample_rate;
    
    // 각 채널 필터 초기화
    for (int i = 0; i < channels; i++) {
        biquad_lowpass(&mf->channels[i], sample_rate, 20000, 0.707); // 20kHz LPF
    }
    
    return mf;
}

void process_multichannel_audio(multichannel_filter_t* mf, double** input, double** output, int num_samples) {
    for (int c = 0; c < mf->num_channels; c++) {
        for (int s = 0; s < num_samples; s++) {
            output[c][s] = biquad_process_simd(&mf->channels[c], input[c][s]);
        }
    }
}
