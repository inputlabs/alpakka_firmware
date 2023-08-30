// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef struct vector_struct {
    double x;
    double y;
    double z;
} Vector;

typedef struct vector4_struct {
    float x;
    float y;
    float z;
    float r;  // Rotation, usually in radians.
} Vector4;

Vector vector_normalize(Vector v);
Vector vector_add(Vector a, Vector b);
Vector vector_sub(Vector a, Vector b);
Vector vector_invert(Vector v);
Vector vector_cross_product(Vector a, Vector b);
Vector vector_smooth(Vector a, Vector b, float factor);
float vector_lenght(Vector v);

Vector4 quaternion(Vector vector, float rotation);
Vector4 qmultiply(Vector4 q1, Vector4 q2);
Vector4 qconjugate(Vector4 q);
Vector qrotate(Vector4 q1, Vector v);
Vector qvector(Vector4 q);

