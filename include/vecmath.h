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

void initVector(Vector *vec, unum n);
void initCopyVector(Vector *dst, const Vector *src);
void resizeVector(Vector *vec, size_t new_size);

void addVectors(Vector *dst, const Vector *a, const Vector *b);
void substractVectors(Vector *dst, const Vector *a, const Vector *b);
void multiplyVectorByValue(Vector *dst, const Vector *a, decimal b);
void divideVectorByValue(Vector *dst, const Vector *a, decimal b);

void copyVector(Vector *dst, const Vector *src);

void setVectorElement(Vector *vec, unum x, decimal new_value);

decimal getVectorElement(Vector *vec, unum x);

void destroyVector(Vector *vec);

typedef struct SqMatrix {
    unum n;
    decimal *data;
} SqMatrix;

void initSqMatrix(SqMatrix *mat, unum n);
void initCopySqMatrix(SqMatrix *mat, const SqMatrix *src);
void initIdentitySqMatrix(SqMatrix *mat, unum n);

void resizeSqMatrix(SqMatrix *matrix, size_t new_size);
void copySqMatrix(SqMatrix *dst, const SqMatrix *src);

void setSqMatrixElement(SqMatrix *matrix, unum x, unum y, decimal new_value);
decimal getSqMatrixElement(const SqMatrix *matrix, unum x, unum y);

void invertMatrix(SqMatrix *matrix);
void transposeMatrix(SqMatrix *matrix);

decimal getDetermitator(const SqMatrix *matrix);
decimal getCofactor(const SqMatrix *matrix, unum x, unum y);

void destroySqMatrix(SqMatrix *matrix);

void multiplyVectorByMatrix(Vector *vec, const SqMatrix *a, const Vector *b);
void multiplyMatrixes(SqMatrix *dst, const SqMatrix *a, const SqMatrix *b);

#ifdef __cplusplus
}
#endif

#endif