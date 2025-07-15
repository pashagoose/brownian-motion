#include "matrix_operations.h"
#include <random>
#include <algorithm>
#include <cmath>

// SIMD headers for different platforms
#ifdef __ARM_NEON
    #include <arm_neon.h>
#elif defined(__SSE2__)
    #include <emmintrin.h>
#endif

void MatrixOperations::slowMatrixMultiply(
    const std::vector<std::vector<float>>& a,
    const std::vector<std::vector<float>>& b,
    std::vector<std::vector<float>>& result) {
    
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        return;
    }
    
    int rows_a = a.size();
    int cols_a = a[0].size();
    int cols_b = b[0].size();
    
    // Resize result matrix
    result.resize(rows_a, std::vector<float>(cols_b, 0.0f));
    
    // INTENTIONALLY SLOW: Bad cache locality
    // We access matrix B in a cache-unfriendly way (column-major instead of row-major)
    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            result[i][j] = 0.0f;
            for (int k = 0; k < cols_a; ++k) {
                // This is the cache-unfriendly access pattern!
                // We're accessing b[k][j] which jumps around in memory
                result[i][j] += a[i][k] * b[k][j];
                
                // Add some extra work to make it even slower and more visible in profiler
                float temp = result[i][j];
                temp = temp * 1.00001f; // Tiny multiplication to add CPU cycles
                result[i][j] = temp;
            }
        }
    }
    
    // Add some pointless additional computation to waste more cycles
    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            // Wasteful sqrt and multiplication operations
            float val = result[i][j];
            val = sqrt(val * val + 0.0001f); // Pointless sqrt
            result[i][j] = val;
        }
    }
}

void MatrixOperations::fastMatrixMultiply(
    const std::vector<std::vector<float>>& a,
    const std::vector<std::vector<float>>& b,
    std::vector<std::vector<float>>& result) {
    
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        return;
    }
    
    int rows_a = a.size();
    int cols_a = a[0].size();
    int cols_b = b[0].size();
    
    // Create transposed version of b for better cache locality
    std::vector<std::vector<float>> b_transposed(cols_b, std::vector<float>(cols_a));
    transposeMatrix(b, b_transposed);
    
    // Resize result matrix
    result.resize(rows_a, std::vector<float>(cols_b, 0.0f));
    
    // CACHE-FRIENDLY multiplication using transposed matrix
    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            result[i][j] = 0.0f;
            // Now we access both a[i] and b_transposed[j] in row-major order
            // This is much more cache-friendly!
            for (int k = 0; k < cols_a; ++k) {
                result[i][j] += a[i][k] * b_transposed[j][k];
            }
        }
    }
}

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

void MatrixOperations::ultraFastMatrixMultiply(
    const std::vector<std::vector<float>>& a,
    const std::vector<std::vector<float>>& b,
    std::vector<std::vector<float>>& result) {
    
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        return;
    }
    
    int rows_a = a.size();
    int cols_a = a[0].size();
    int cols_b = b[0].size();
    
    // Resize result matrix
    result.resize(rows_a, std::vector<float>(cols_b, 0.0f));
    
    // Create transposed version of b for better cache locality
    std::vector<std::vector<float>> b_transposed(cols_b, std::vector<float>(cols_a));
    transposeMatrix(b, b_transposed);
    
    // SIMD-optimized multiplication
    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            float sum = 0.0f;
            int k = 0;
            
#ifdef __ARM_NEON
            // ARM NEON SIMD optimization
            float32x4_t sum_vec = vdupq_n_f32(0.0f);
            
            // Process 4 elements at a time
            for (; k <= cols_a - 4; k += 4) {
                float32x4_t a_vec = vld1q_f32(&a[i][k]);
                float32x4_t b_vec = vld1q_f32(&b_transposed[j][k]);
                sum_vec = vfmaq_f32(sum_vec, a_vec, b_vec);
            }
            
            // Sum all elements in the vector
            float32x2_t sum_pair = vadd_f32(vget_low_f32(sum_vec), vget_high_f32(sum_vec));
            sum = vget_lane_f32(vpadd_f32(sum_pair, sum_pair), 0);
            
#elif defined(__SSE2__)
            // x86 SSE2 SIMD optimization
            __m128 sum_vec = _mm_setzero_ps();
            
            // Process 4 elements at a time
            for (; k <= cols_a - 4; k += 4) {
                __m128 a_vec = _mm_loadu_ps(&a[i][k]);
                __m128 b_vec = _mm_loadu_ps(&b_transposed[j][k]);
                sum_vec = _mm_add_ps(sum_vec, _mm_mul_ps(a_vec, b_vec));
            }
            
            // Sum all elements in the vector
            sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
            sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
            sum = _mm_cvtss_f32(sum_vec);
#endif
            
            // Handle remaining elements
            for (; k < cols_a; ++k) {
                sum += a[i][k] * b_transposed[j][k];
            }
            
            result[i][j] = sum;
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