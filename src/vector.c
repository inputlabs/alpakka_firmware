// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <math.h>
#include "vector.h"

Vector2 vector2_normalize(Vector2 v) {
    double mag = (v.x*v.x) + (v.y*v.y);
    if (fabs(mag - 1.0) > 0.0001) {  // Tolerance.
        mag = sqrt(mag);
        return (Vector2){v.x/mag, v.y/mag};
    }
    return v;
}

Vector vector_normalize(Vector v) {
    double mag = (v.x*v.x) + (v.y*v.y) + (v.z*v.z);
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

Vector vector_multiply(Vector v, double n) {
    return (Vector){v.x*n, v.y*n, v.z*n};
}

Vector vector_negative(Vector v) {
    return (Vector){-v.x, -v.y, -v.z};
}

Vector vector_cross_product(Vector a, Vector b) {
    return (Vector){
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x)
    };
}

Vector vector_smooth(Vector a, Vector b, double factor) {
    return  (Vector){
        (a.x*factor + b.x) / (factor + 1),
        (a.y*factor + b.y) / (factor + 1),
        (a.z*factor + b.z) / (factor + 1)
    };
}

double vector_lenght(Vector v) {
    return sqrt(
        powf(fabs(v.x), 2) +
        powf(fabs(v.y), 2) +
        powf(fabs(v.z), 2)
    );
}

Vector4 quaternion(Vector vector, double rotation) {
    vector = vector_normalize(vector);
    double theta = rotation / 2;
    return (Vector4){
        vector.x * sin(theta),
        vector.y * sin(theta),
        vector.z * sin(theta),
        cos(theta)
    };
}

Vector4 qmultiply(Vector4 q1, Vector4 q2) {
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

