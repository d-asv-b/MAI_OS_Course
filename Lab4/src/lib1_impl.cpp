#include "../include/lib_1.hpp"
#include <cmath>

// First library implementation

extern "C" {
    float Pi(int K) {
        float sum = 0.0f;
        for (int i = 0; i < K; i++) {
            float term = 1.0f / (2 * i + 1);
            if (i % 2 == 0) {
                sum += term;
            } else {
                sum -= term;
            }
        }
        return 4.0f * sum;
    }

    float E(int x) {
        if (x == 0) return 1.0f;
        return std::pow(1.0f + 1.0f / x, x);
    }
}
