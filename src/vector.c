// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <math.h>
#include "vector.h"

Vector vector_normalize(Vector v) {
    float mag = (v.x*v.x) + (v.y*v.y) + (v.z*v.z);
    if (fabs(mag - 1.0) > 0.0001) {  // Tolerance.
        mag = sqrt(mag);
        return (Vector){v.x/mag, v.y/mag, v.z/mag};
    }
    return v;
}

Vector vector_add(Vector a, Vector b) {
    return (Vector){a.x+b.x, a.y+b.y, a.z+b.z};
}

Vector vector_sub(Vector a, Vector b) {
    return (Vector){a.x-b.x, a.y-b.y, a.z-b.z};
}

Vector vector_invert(Vector v) {
    return (Vector){-v.x, -v.y, -v.z};
}

Vector vector_cross_product(Vector a, Vector b) {
    return (Vector){
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x)
    };
}

// Get a pseudo-rolling average of A and B according to given weight.
// Only (1/weight) parts of B is incorporated into A.
// The higher the weight the more averaged the result is.
Vector vector_smooth(Vector a, Vector b, float weight) {
    return  (Vector){
        (a.x*weight + b.x) / (weight + 1),
        (a.y*weight + b.y) / (weight + 1),
        (a.z*weight + b.z) / (weight + 1)
    };
}

float vector_lenght(Vector v) {
    return sqrt(
        powf(fabs(v.x), 2) +
        powf(fabs(v.y), 2) +
        powf(fabs(v.z), 2)
    );
}

Vector4 quaternion(Vector vector, float rotation /*radians*/) {
    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    vector = vector_normalize(vector);
    float theta = rotation / 2;
    return (Vector4){
        vector.x * sin(theta),
        vector.y * sin(theta),
        vector.z * sin(theta),
        cos(theta)
    };
}

Vector4 qmultiply(Vector4 q1, Vector4 q2) {
    // https://en.wikipedia.org/wiki/Quaternion
    return (Vector4){
        q1.r * q2.x + q1.x * q2.r + q1.y * q2.z - q1.z * q2.y,
        q1.r * q2.y + q1.y * q2.r + q1.z * q2.x - q1.x * q2.z,
        q1.r * q2.z + q1.z * q2.r + q1.x * q2.y - q1.y * q2.x,
        q1.r * q2.r - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
    };
}

Vector4 qconjugate(Vector4 q) {
    return (Vector4){-q.x, -q.y, -q.z, q.r};
}

Vector qrotate(Vector4 q1, Vector v) {
    Vector4 q2 = (Vector4){v.x, v.y, v.z, 0};
    return qvector(qmultiply(qmultiply(q1, q2), qconjugate(q1)));
}

Vector qvector(Vector4 q) {
    return vector_normalize((Vector){q.x, q.y, q.z});
}

