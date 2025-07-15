#include "matrix_operations.h"
#include <random>
#include <algorithm>
#include <cmath>

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
    
    // INTENTIONALLY SLOW: Bad cache locality
    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            result[i][j] = 0.0f;
            for (int k = 0; k < cols_a; ++k) {
                result[i][j] += a[i][k] * b[k][j];
                
                float temp = result[i][j];
                temp = temp * 1.00001f;
                result[i][j] = temp;
            }
        }
    }
    
    for (int i = 0; i < rows_a; ++i) {
        for (int j = 0; j < cols_b; ++j) {
            float val = result[i][j];
            val = sqrt(val * val + 0.0001f);
            result[i][j] = val;
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

// --- DEFAULT (NON-OPTIMIZED) IMPLEMENTATION ---
#else

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