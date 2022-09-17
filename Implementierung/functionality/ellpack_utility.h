#ifndef ELLPACK_UTILITY_H
#define ELLPACK_UTILITY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <error.h>
#include <emmintrin.h>
#include <immintrin.h>

/** the struct storing the representation matrices and dimensions */
struct EllpackMatrix {
    u_int64_t real_width;
    u_int64_t height;
    u_int64_t width;
    float *values;
    u_int64_t *indices;
};

/** creates an EllpackMatrix with the representation matrices with given dimensions,
 * initialised to 0, errors are printed to the given file */
struct EllpackMatrix *make_ellpack(u_int64_t real_width, u_int64_t height, u_int64_t width, char* file);

/** deallocates any dynamic memory associated with the EllpackMatrix */
void free_ellpack(struct EllpackMatrix *x);

/** deallocates a given array of matrices */
void free_all(struct EllpackMatrix *matrices[], int n);

/** checks whether the struct and the representation matrices are allocated */
int valid_ellpack(const struct EllpackMatrix *x);

/** prints a named matrix to the given file */
void print_ellpack(FILE *output, struct EllpackMatrix *ellpack_matrix, char* name);

/** finds the number of the rightmost column with non zero entries - the width of the narrowest real matrix */
u_int64_t realwidth_ellpack(const struct EllpackMatrix *x);

/** creates the representation matrices in the ellpack matrix from the arrays of rows */
void flatten_ellpack(struct EllpackMatrix *x, float **values, u_int64_t **indices, u_int64_t *lengths);

/** creates and returns a the transpose of the matrix in ellpack format */
struct EllpackMatrix *transpose_ellpack(const struct EllpackMatrix * x);

/** adds the fours floats in a 128 bit register */
float hsum_ps_sse1(__m128 v);

#endif