#include <math.h>
#include "contract.h"

float sin_integral(float a, float b, float e) {
    if (a > b) {
        float temp = a;
        a = b;
        b = temp;
    }
    
    float sum = 0.0f;
    float x = a;
    while (x < b) {
        float step = (x + e <= b) ? e : (b - x);
        sum += sinf(x) * step;
        x += step;
    }
    return sum;
}
