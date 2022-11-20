/*
 * file: vecmath.h
 * author: filebin
 * date: 2022-11-20 14:49:44
 */

#ifndef INCLUDE_VECMATH_H_
#define INCLUDE_VECMATH_H_

#include "defines.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Vector {
    unum n;
    decimal *data;
} Vector;

Vector createVector(unum n);

void assignVector(Vector *dst, const Vector *src);

void setVectorElement(Vector *vec, unum x, decimal new_value);

decimal getVectorElement(Vector *vec, unum x);

void destroyVector(Vector *vec);

typedef struct SqMatrix {
    unum n;
    decimal *data;
} SqMatrix;

SqMatrix createSqMatrix(unum n);
SqMatrix createCopyMatrix(const SqMatrix *src);

SqMatrix createIdentitySqMatrix(unum n);

void resizeSqMatrix(SqMatrix *matrix, size_t new_size);
void copySqMatrix(SqMatrix *dst, const SqMatrix *src);

void setSqMatrixElement(SqMatrix *matrix, unum x, unum y, decimal new_value);
decimal getSqMatrixElement(SqMatrix *matrix, unum x, unum y);

void invertMatrix(SqMatrix *matrix);
void transposeMatrix(SqMatrix *matrix);

decimal getDetermitator(const SqMatrix *matrix);
decimal getCofactor(const SqMatrix *matrix, unum x, unum y);

void destroySqMatrix(SqMatrix *matrix);

#ifdef __cplusplus
}
#endif

#endif