#include "defines.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vecmath.h>

Vector createVector(unum n) {
    Vector v;
    v.n = n;
    v.data = ALLOC_ARR(decimal, n);
    ZERO_ARR(v.data, decimal, n);
    return v;
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