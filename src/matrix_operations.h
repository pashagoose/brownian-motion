#pragma once

#include <vector>

class MatrixOperations {
public:
    // Intentionally slow matrix multiplication for demonstration
    // This will show up prominently in the profiler
    static void slowMatrixMultiply(
        const std::vector<std::vector<float>>& a,
        const std::vector<std::vector<float>>& b,
        std::vector<std::vector<float>>& result
    );
    
    // Optimized version (cache-friendly with transposition)
    static void fastMatrixMultiply(
        const std::vector<std::vector<float>>& a,
        const std::vector<std::vector<float>>& b,
        std::vector<std::vector<float>>& result
    );
    
    // Ultra-fast version with SIMD optimizations for ARM/x86
    static void ultraFastMatrixMultiply(
        const std::vector<std::vector<float>>& a,
        const std::vector<std::vector<float>>& b,
        std::vector<std::vector<float>>& result
    );
    
    // Helper functions
    static void createIdentityMatrix(std::vector<std::vector<float>>& matrix, int size);
    static void createRandomMatrix(std::vector<std::vector<float>>& matrix, int rows, int cols);
    static void transposeMatrix(
        const std::vector<std::vector<float>>& input,
        std::vector<std::vector<float>>& output
    );
}; 