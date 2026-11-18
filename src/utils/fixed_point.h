#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>

// Use 2 decimal places for currency (e.g., 100 = 1.00)
#define SCALE 100

typedef int64_t Amount;

static inline Amount to_fixed(double value) {
    return (Amount)(value * SCALE);
}

static inline double from_fixed(Amount value) {
    return (double)value / SCALE;
}

#endif // FIXED_POINT_H
