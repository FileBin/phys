#include "defines.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vecmath.h>

void initVector(Vector *v, unum n) {
    v->n = n;
    v->data = ALLOC_ARR(decimal, n);
    ZERO_ARR(v->data, decimal, n);
}

void initCopyVector(Vector *dst, const Vector *src) {
    dst->n = src->n;
    dst->data = ALLOC_ARR(decimal, src->n);
    memcpy(dst->data, src->data, src->n * sizeof(decimal));
}

void resizeVector(Vector *vec, size_t new_size) {
    vec->data = reallocarray(vec->data, new_size, sizeof(decimal));
    if (new_size > vec->n) {
        memset(vec->data + vec->n, 0, (new_size - vec->n) * sizeof(decimal));
    }
    vec->n = new_size;
}

void copyVector(Vector *dst, const Vector *src) {
    if (dst->n != src->n) {
        resizeVector(dst, src->n);
    }
    dst->n = src->n;
    memcpy(dst->data, src->data, src->n * sizeof(decimal));
}

void setVectorElement(Vector *vec, unum x, decimal new_value) { vec->data[x] = new_value; }

decimal getVectorElement(Vector *vec, unum x) { return vec->data[x]; }

void destroyVector(Vector *vec) {
    vec->n = 0;
    SAFE_FREE(vec->data);
}

void addVectors(Vector *dst, const Vector *a, const Vector *b) {
    unum n = dst->n;
    for (size_t i = 0; i < n; ++i) {
        dst->data[i] = a->data[i] + b->data[i];
    }
}

void substractVectors(Vector *dst, const Vector *a, const Vector *b) {
    unum n = dst->n;
    for (size_t i = 0; i < n; ++i) {
        dst->data[i] = a->data[i] - b->data[i];
    }
}

void multiplyVectorByValue(Vector *dst, const Vector *a, decimal b) {
    unum n = dst->n;
    for (size_t i = 0; i < n; ++i) {
        dst->data[i] = a->data[i] * b;
    }
}

void divideVectorByValue(Vector *dst, const Vector *a, decimal b) {
    unum n = dst->n;
    for (size_t i = 0; i < n; ++i) {
        dst->data[i] = a->data[i] / b;
    }
}

void multiplyVectorByMatrix(Vector *dst, const SqMatrix *a, const Vector *b) {
    unum n = dst->n;
    for (size_t i = 0; i < n; ++i) {
        dst->data[i] = 0;
        for (size_t j = 0; j < n; ++j) {
            dst->data[i] += b->data[j] * getSqMatrixElement(a, j, i);
        }
    }
}