#include "defines.h"
#include <assert.h>
#include <string.h>
#include <vecmath.h>

#define MATRIX_ELEM(matrix, x, y) (matrix)->data[(x) + (y) * (matrix)->n]

void initSqMatrix(SqMatrix *matrix, unum n) {
    matrix->n = n;
    matrix->data = ALLOC_ARR(decimal, n * n);
    ZERO_ARR(matrix->data, decimal, n * n);
}

void initIdentitySqMatrix(SqMatrix *matrix, unum n) {
    initIdentitySqMatrix(matrix, n);
    for (size_t i = 0; i < n; ++i) {
        MATRIX_ELEM(matrix, i, i) = 1;
    }
}

void resizeSqMatrix(SqMatrix *matrix, size_t new_size) {
    matrix->data = reallocarray(matrix->data, new_size * new_size, sizeof(decimal));
    if (new_size > matrix->n) {
        unum offset = matrix->n * matrix->n;
        memset(matrix->data + offset, 0, (new_size * new_size - offset) * sizeof(decimal));
    }
    matrix->n = new_size;
}

void initCopySqMatrix(SqMatrix *dst, const SqMatrix *src) {
    initSqMatrix(dst, src->n);
    memcpy(dst->data, src->data, src->n * src->n * sizeof(decimal));
}

void copySqMatrix(SqMatrix *dst, const SqMatrix *src) {
    if (dst->n != src->n) {
        resizeSqMatrix(dst, src->n);
    }
    dst->n = src->n;
    memcpy(dst->data, src->data, src->n * src->n * sizeof(decimal));
}

void setSqMatrixElement(SqMatrix *matrix, unum x, unum y, decimal new_value) { MATRIX_ELEM(matrix, x, y) = new_value; }

decimal getSqMatrixElement(SqMatrix *matrix, unum x, unum y) { return MATRIX_ELEM(matrix, x, y); }

void transposeMatrix(SqMatrix *matrix) {
    unum n = matrix->n;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            decimal t = MATRIX_ELEM(matrix, i, j);
            MATRIX_ELEM(matrix, i, j) = MATRIX_ELEM(matrix, j, i);
            MATRIX_ELEM(matrix, j, i) = t;
        }
    }
}

void invertMatrix(SqMatrix *matrix) {
    unum n = matrix->n;
    if (n == 1) {
        matrix->data[0] = 1 / matrix->data[0];
    } else {
        transposeMatrix(matrix);
        SqMatrix copy;
        initCopySqMatrix(&copy, matrix);

        decimal det = 0;
        for (size_t i = 0; i < n; ++i) {
            decimal cof = getCofactor(&copy, 0, i);
            det += MATRIX_ELEM(&copy, 0, i) * cof;
            MATRIX_ELEM(matrix, 0, i) = cof;
        }

        for (size_t i = 0; i < n; ++i) {
            MATRIX_ELEM(matrix, 0, i) /= det;
        }

        for (size_t i = 1; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                MATRIX_ELEM(matrix, j, i) = getCofactor(&copy, j, i) / det;
            }
        }
        destroySqMatrix(&copy);
    }
}

decimal getDetermitator(const SqMatrix *matrix) {
    decimal sum;
    switch (matrix->n) {
    case 1:
        return matrix->data[0];
    case 2:
        return matrix->data[0] * matrix->data[3] - matrix->data[1] * matrix->data[2];
    default:
        sum = 0;
        for (size_t i = 0; i < matrix->n; ++i) {
            sum += MATRIX_ELEM(matrix, 0, i) * getCofactor(matrix, 0, i);
        }
        return sum;
    }
}

decimal getCofactor(const SqMatrix *matrix, unum x, unum y) {
    unum n = matrix->n;
    assert(n > 1);
    if (n == 2) {
        return MATRIX_ELEM(matrix, 1 - x, 1 - y);
    }
    SqMatrix cofactor_matrix;
    initSqMatrix(&cofactor_matrix, n - 1);
    decimal *dst = cofactor_matrix.data;
    decimal *src = matrix->data;
    for (size_t i = 0; i < n; ++i) {
        if (i == y) {
            src = src + n;
            continue;
        }

        unum s = x;

        if (s > 0)
            memcpy(dst, src, s * sizeof(decimal));

        s = n - x - 1;

        dst = dst + x;
        src = src + x + 1;

        if (s > 0)
            memcpy(dst, src, s * sizeof(decimal));

        dst = dst + (n - 1 - x);
        src = src + (n - 1 - x);
    }
    decimal det = getDetermitator(&cofactor_matrix);
    destroySqMatrix(&cofactor_matrix);
    return (x + y) % 2 ? -det : det;
}

void destroySqMatrix(SqMatrix *matrix) {
    matrix->n = 0;
    SAFE_FREE(matrix->data);
}