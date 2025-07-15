#pragma once

#include <vector>

class MatrixOperations {
public:
    // Single matrix multiplication function - implementation varies based on build target
    // This ensures consistent naming in profiler flame graphs across all optimization levels
    static void multiplyMatrices(
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