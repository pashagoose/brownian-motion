#include "matrix_operations.h"
#include <random>
#include <algorithm>
#include <cmath>

// SIMD headers for ultra-fast implementation
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    #include <arm_neon.h>
    #define USE_NEON
#elif defined(__SSE2__) || defined(__SSE3__) || defined(__SSE4_1__) || defined(__AVX__)
    #include <immintrin.h>
    #define USE_SSE
#endif

// Auto-select slow implementation if no flag is specified
#if !defined(USE_SLOW_MATRIX) && !defined(USE_FAST_MATRIX) && !defined(USE_ULTRA_FAST_MATRIX)
    #define USE_SLOW_MATRIX
#endif

// --- SLOW IMPLEMENTATION ---
#if defined(USE_SLOW_MATRIX)

void MatrixOperations::multiplyMatrices(
    const std::vector<std::vector<float>>& a,
    const std::vector<std::vector<float>>& b,
    std::vector<std::vector<float>>& result) {

    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        return;
    }
    
    int rows_a = a.size();
    int cols_a = a[0].size();
    int cols_b = b[0].size();

    result.resize(rows_a, std::vector<float>(cols_b, 0.0f));

    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            result[i][j] = 0.0f;
            for (int k = 0; k < cols_a; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// --- FAST IMPLEMENTATION ---
#elif defined(USE_FAST_MATRIX)

void MatrixOperations::multiplyMatrices(
    const std::vector<std::vector<float>>& a,
    const std::vector<std::vector<float>>& b,
    std::vector<std::vector<float>>& result) {
    
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        return;
    }
    
    int rows_a = a.size();
    int cols_a = a[0].size();
    int cols_b = b[0].size();
    
    std::vector<std::vector<float>> b_transposed(cols_b, std::vector<float>(cols_a));
    transposeMatrix(b, b_transposed);
    
    result.resize(rows_a, std::vector<float>(cols_b, 0.0f));
    
    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            result[i][j] = 0.0f;
            for (int k = 0; k < cols_a; ++k) {
                result[i][j] += a[i][k] * b_transposed[j][k];
            }
        }
    }
}

// --- ULTRA FAST IMPLEMENTATION (SIMD + Cache Optimization) ---
#elif defined(USE_ULTRA_FAST_MATRIX)

void MatrixOperations::multiplyMatrices(
    const std::vector<std::vector<float>>& a,
    const std::vector<std::vector<float>>& b,
    std::vector<std::vector<float>>& result) {
    
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        return;
    }
    
    int rows_a = a.size();
    int cols_a = a[0].size();
    int cols_b = b[0].size();
    
    // Transpose matrix B for better cache locality
    std::vector<std::vector<float>> b_transposed(cols_b, std::vector<float>(cols_a));
    transposeMatrix(b, b_transposed);
    
    result.resize(rows_a, std::vector<float>(cols_b, 0.0f));
    
    // Block size optimized for cache performance
    const int block_size = 64;
    
    // Process in blocks for cache efficiency
    for (int ii = 0; ii < rows_a; ii += block_size) {
        for (int jj = 0; jj < cols_b; jj += block_size) {
            int max_i = std::min(ii + block_size, rows_a);
            int max_j = std::min(jj + block_size, cols_b);
            
            // Inner block multiplication with SIMD
            for (int i = ii; i < max_i; ++i) {
                for (int j = jj; j < max_j; ++j) {
                    
#if defined(USE_NEON)
                    // ARM NEON implementation
                    float32x4_t sum_vec = vdupq_n_f32(0.0f);
                    int k = 0;
                    
                    // Process 4 elements at a time
                    for (; k <= cols_a - 4; k += 4) {
                        float32x4_t a_vec = vld1q_f32(&a[i][k]);
                        float32x4_t b_vec = vld1q_f32(&b_transposed[j][k]);
                        sum_vec = vmlaq_f32(sum_vec, a_vec, b_vec);
                    }
                    
                    // Sum the vector components manually for better compatibility
                    float32x2_t sum_pair = vadd_f32(vget_low_f32(sum_vec), vget_high_f32(sum_vec));
                    float sum = vget_lane_f32(vpadd_f32(sum_pair, sum_pair), 0);
                    
                    // Handle remaining elements
                    for (; k < cols_a; ++k) {
                        sum += a[i][k] * b_transposed[j][k];
                    }
                    
                    result[i][j] = sum;
                    
#elif defined(USE_SSE)
                    // Intel SSE implementation
                    __m128 sum_vec = _mm_setzero_ps();
                    int k = 0;
                    
                    // Process 4 elements at a time
                    for (; k <= cols_a - 4; k += 4) {
                        __m128 a_vec = _mm_loadu_ps(&a[i][k]);
                        __m128 b_vec = _mm_loadu_ps(&b_transposed[j][k]);
                        sum_vec = _mm_add_ps(sum_vec, _mm_mul_ps(a_vec, b_vec));
                    }
                    
                    // Sum the vector components
                    float sum_array[4];
                    _mm_storeu_ps(sum_array, sum_vec);
                    float sum = sum_array[0] + sum_array[1] + sum_array[2] + sum_array[3];
                    
                    // Handle remaining elements
                    for (; k < cols_a; ++k) {
                        sum += a[i][k] * b_transposed[j][k];
                    }
                    
                    result[i][j] = sum;
                    
#else
                    // Fallback: manual unrolling without SIMD
                    float sum = 0.0f;
                    int k = 0;
                    
                    // Unroll by 4 for better performance
                    for (; k <= cols_a - 4; k += 4) {
                        sum += a[i][k] * b_transposed[j][k] +
                               a[i][k+1] * b_transposed[j][k+1] +
                               a[i][k+2] * b_transposed[j][k+2] +
                               a[i][k+3] * b_transposed[j][k+3];
                    }
                    
                    // Handle remaining elements
                    for (; k < cols_a; ++k) {
                        sum += a[i][k] * b_transposed[j][k];
                    }
                    
                    result[i][j] = sum;
#endif
                }
            }
        }
    }
}

#endif

void MatrixOperations::createIdentityMatrix(std::vector<std::vector<float>>& matrix, int size) {
    matrix.resize(size, std::vector<float>(size, 0.0f));
    
    for (int i = 0; i < size; ++i) {
        matrix[i][i] = 1.0f;
    }
}

void MatrixOperations::createRandomMatrix(std::vector<std::vector<float>>& matrix, int rows, int cols) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    matrix.resize(rows, std::vector<float>(cols));
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dist(gen);
        }
    }
}

void MatrixOperations::transposeMatrix(
    const std::vector<std::vector<float>>& input,
    std::vector<std::vector<float>>& output) {
    
    if (input.empty()) {
        return;
    }
    
    int rows = input.size();
    int cols = input[0].size();
    
    output.resize(cols, std::vector<float>(rows));
    
    // Блочное транспонирование для лучшей кэш-локальности
    const int block_size = 32;
    
    for (int i = 0; i < rows; i += block_size) {
        for (int j = 0; j < cols; j += block_size) {
            int max_i = std::min(i + block_size, rows);
            int max_j = std::min(j + block_size, cols);
            
            for (int ii = i; ii < max_i; ++ii) {
                for (int jj = j; jj < max_j; ++jj) {
                    output[jj][ii] = input[ii][jj];
                }
            }
        }
    }
}