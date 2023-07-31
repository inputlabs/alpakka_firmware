// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef struct vector_struct {
    double x;
    double y;
    double z;
} Vector;

typedef struct Vector4_struct {
    double x;
    double y;
    double z;
    double r;
} Vector4;

Vector vector_normalize(Vector v);
Vector vector_add(Vector a, Vector b);
Vector vector_sub(Vector a, Vector b);
Vector vector_multiply(Vector v, double n);
Vector vector_cross_product(Vector a, Vector b);
Vector vector_mix(Vector a, Vector b, double factor);
Vector vector_smooth(Vector a, Vector b, double factor);
double vector_lenght(Vector v);

Vector4 quartenion(Vector vector, double rotation);
Vector4 qmultiply(Vector4 q1, Vector4 q2);
Vector4 qconjugate(Vector4 q);
Vector4 qrotate(Vector4 q1, Vector v);
Vector qvector(Vector4 q);
