#include "../include/lib_2.hpp"
#include <cmath>

// Second library implementation

extern "C" {
    float Pi(int K) {
        float product = 1.0f;
        for (int i = 1; i <= K; i++) {
            product *= (2.0f * i) / (2.0f * i - 1.0f);
            product *= (2.0f * i) / (2.0f * i + 1.0f);
        }
        return 2.0f * product;
    }

    float E(int x) {
        float sum = 0.0f;
        float factorial = 1.0f;
        
        for (int n = 0; n <= x; n++) {
            if (n > 0) {
                factorial *= n;
            }
            sum += 1.0f / factorial;
        }
        
        return sum;
    }
}
