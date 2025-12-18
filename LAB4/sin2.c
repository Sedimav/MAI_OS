#include <math.h>
#include "contract.h"

float sin_integral(float a, float b, float e) {
    if (a > b) {
        float temp = a;
        a = b;
        b = temp;
    }
    
    int n = (int)((b - a) / e) + 1;
    float h = (b - a) / n;
    float sum = 0.5f * (sinf(a) + sinf(b));
    
    for (int i = 1; i < n; i++) {
        float x = a + i * h;
        sum += sinf(x);
    }
    
    return sum * h;
}
