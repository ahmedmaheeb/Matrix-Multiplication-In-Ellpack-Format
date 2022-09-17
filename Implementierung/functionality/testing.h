//
// Created by abdelgha on 7/8/22.
//

#ifndef PROJEKTAUFGABE_TESTING_H
#define PROJEKTAUFGABE_TESTING_H

#include "multiplication.h"

#include <stdbool.h>

struct TestStruct {
    struct EllpackMatrix* a;
    struct EllpackMatrix* b;
    struct EllpackMatrix* r;
};

enum TestMatrices {
    DIM1X1V1, DIM1X1V2, DIM1X1V3, DIM1X1V6,
    DIM2X2V1, DIM2X2V2, DIM2X2V3, DIM2X2V6,
    DIM3X3V1, DIM3X3V2, DIM3X3V3, DIM3X3V6,
    BIGSPARSE2X2A, BIGSPARSE2X2B, BIGSPARSE2X2R,
    BIGSPARSE4X4A, BIGSPARSE4X4B, BIGSPARSE4X4R,
    BIGSPARSE4X5A, BIGSPARSE5X6B, BIGSPARSE4X6R
};

enum TestCases {
    IDENTITY1, IDENTITY2, IDENTITY3,
    VALUES1, VALUES2, VALUES3,
    BIGSPARSE1, BIGSPARSE2, BIGSPARSE3,
    TERMINAL
};

extern const float TESTING_PRECISION;

struct EllpackMatrix *create_ellpack(u_int64_t rw, u_int64_t w, u_int64_t h, const float values[], const u_int64_t indices[]);

struct EllpackMatrix *choose_testmatrix(enum TestMatrices test_matrix);

struct TestStruct choose_testcase(enum TestCases test_case);

bool compare_ellpack(struct EllpackMatrix *a, struct EllpackMatrix *b);

void testing(enum MultVersion version, FILE *report);

#endif //PROJEKTAUFGABE_TESTING_H
