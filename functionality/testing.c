//
// Created by Ahmed on 7/8/22.
//

#include "testing.h"

#include <stdio.h>
#include <time.h>

struct EllpackMatrix *create_ellpack(u_int64_t rw, u_int64_t w, u_int64_t h, const float values[], const u_int64_t indices[]) {
    struct EllpackMatrix * r = make_ellpack(rw, h, w, "");
    for (u_int64_t i = 0; i < r->height; i++) {
        for (u_int64_t j = 0; j < r->width; j++) {
            *(r->values + i * r->width + j) = values[i * r->width + j];
            *(r->indices + i * r->width + j) = indices[i * r->width + j];
        }
    }
    return r;
}

struct EllpackMatrix *choose_ellpack(enum TestMatrices test_matrix) {
    switch (test_matrix) {
    case DIM1X1V1: return create_ellpack(1,1,1,(float[]){1.0F},(u_int64_t[]){0});
    case DIM1X1V2: return create_ellpack(1,1,1,(float[]){1.3F},(u_int64_t[]){0});
    case DIM1X1V3: return create_ellpack(1,1,1,(float[]){1.4F},(u_int64_t[]){0});
    case DIM1X1V6: return create_ellpack(1,1,1,(float[]){1.82F},(u_int64_t[]){0});
    case DIM2X2V1: return create_ellpack(2,1,2,(float[]){1.0F,1.0F},(u_int64_t[]){0,1});
    case DIM2X2V2: return create_ellpack(2,2,2,(float[]){2.1F,3.4F,-5.7F,6.3F},(u_int64_t[]){0,1,0,1});
    case DIM2X2V3: return create_ellpack(2,2,2,(float[]){-1.1F,-5.0F,4.4F,-0.5F},(u_int64_t[]){0,1,0,1});
    case DIM2X2V6: return create_ellpack(2,2,2,(float[]){12.65F,-12.2F,33.99F,25.35F},(u_int64_t[]){0,1,0,1});
    case DIM3X3V1: return create_ellpack(3,1,3,(float[]){1.0F,1.0F,1.0F},(u_int64_t[]){0,1,2});
    case DIM3X3V2: return create_ellpack(3,3,3,(float[]){-1.1F,0.7F,0.8F,1.4F,5.6F,-4.4F,2.1F,2.3F,2.2F},(u_int64_t[]){0,1,2,0,1,2,0,1,2});
    case DIM3X3V3: return create_ellpack(3,3,3,(float[]){2.2F,5.5F,110.1F,2.3F,1.1F,2.1F,222.2F,6.6F,7.7F},(u_int64_t[]){0,1,2,0,1,2,0,1,2});
    case DIM3X3V6: return create_ellpack(3,3,3,(float[]){176.95F,-113.48F,0,-961.72F,-15.18F,132.02F,498.75F,28.6F,252.98F},(u_int64_t[]){0,2,0,0,1,2,0,1,2});
    case BIGSPARSE2X2A: return create_ellpack(2,2,2,(float[]){2,-1,2,0},(u_int64_t[]){0,1,0,0});
    case BIGSPARSE2X2B: return create_ellpack(2,2,2,(float[]){1,4,2,0},(u_int64_t[]){0,1,0,0});
    case BIGSPARSE2X2R: return create_ellpack(2,2,2,(float[]){8,0,2,8},(u_int64_t[]){1,0,0,1});
    case BIGSPARSE4X4A: return create_ellpack(4,2,4,(float[]){3,2,11,0,17,13,5,7},(u_int64_t[]){0,2,1,0,2,3,0,1});
    case BIGSPARSE4X4B: return create_ellpack(4,1,4,(float[]){2,7,-3,13},(u_int64_t[]){1,0,1,3});
    case BIGSPARSE4X4R: return create_ellpack(4,2,4,(float[]){0,0,77,0,-51,169,49,10},(u_int64_t[]){0,0,0,0,1,3,0,1});
    case BIGSPARSE4X5A: return create_ellpack(5,2,4,(float[]){2,11,3,13,5,0,19,7},(u_int64_t[]){0,3,1,4,2,0,0,3});
    case BIGSPARSE5X6B: return create_ellpack(6,3,5,(float[]){3,2,1,3,2,1,3,2,1,3,2,1,3,2,1},(u_int64_t[]){1,3,4,1,2,4,1,3,5,1,2,4,1,3,4});
    case BIGSPARSE4X6R: return create_ellpack(6,4,4,(float[]){39,22,4,13,48,6,26,16,15,10,5,0,78,14,38,26},(u_int64_t[]){1,2,3,4,1,2,3,4,1,3,5,0,1,2,3,4});
    default: return NULL;
    }
}

struct TestStruct choose_testcase(enum TestCases test_case) {
    switch(test_case) {
        case IDENTITY1: return (struct TestStruct){choose_ellpack(DIM1X1V1), choose_ellpack(DIM1X1V1), choose_ellpack(DIM1X1V1)};
        case IDENTITY2: return (struct TestStruct){choose_ellpack(DIM2X2V1), choose_ellpack(DIM2X2V1), choose_ellpack(DIM2X2V1)};
        case IDENTITY3: return (struct TestStruct){choose_ellpack(DIM3X3V1), choose_ellpack(DIM3X3V1), choose_ellpack(DIM3X3V1)};
        case VALUES1: return (struct TestStruct){choose_ellpack(DIM1X1V2), choose_ellpack(DIM1X1V3), choose_ellpack(DIM1X1V6)};
        case VALUES2: return (struct TestStruct){choose_ellpack(DIM2X2V2), choose_ellpack(DIM2X2V3), choose_ellpack(DIM2X2V6)};
        case VALUES3: return (struct TestStruct){choose_ellpack(DIM3X3V2), choose_ellpack(DIM3X3V3), choose_ellpack(DIM3X3V6)};
        case BIGSPARSE1: return (struct TestStruct){choose_ellpack(BIGSPARSE2X2A), choose_ellpack(BIGSPARSE2X2B), choose_ellpack(BIGSPARSE2X2R)};
        case BIGSPARSE2: return (struct TestStruct){choose_ellpack(BIGSPARSE4X4A), choose_ellpack(BIGSPARSE4X4B), choose_ellpack(BIGSPARSE4X4R)};
        case BIGSPARSE3: return (struct TestStruct){choose_ellpack(BIGSPARSE4X5A), choose_ellpack(BIGSPARSE5X6B), choose_ellpack(BIGSPARSE4X6R)};
        default: return (struct TestStruct){0, 0, 0};
    }
}

const float TESTING_PRECISION = 0.0001f; // enough to check every test value

bool compare_ellpack(struct EllpackMatrix *a, struct EllpackMatrix *b) {
    bool equal = a -> width == b -> width && a -> height == b -> height;
    if (!equal) {
        return false;
    }
    for (u_int64_t i = 0; i < a -> width * a -> height; i++) {
        equal = equal && fabsf(a -> values[i] - b -> values[i]) < TESTING_PRECISION && a -> indices[i] == b -> indices[i];
    }
    return equal;
}

void testing(enum MultVersion version, FILE *report) {
    for (enum TestCases test_case = 0; test_case != TERMINAL; test_case++) {
        struct TestStruct test = choose_testcase(test_case);
        struct EllpackMatrix *res = malloc(sizeof(*res));
        switch (version) {
            case LINEAR:
                matr_mult_ellpack(test.a, test.b, res);
                break;
            case VECTORIZED:
                matr_mult_ellpack_vectorised(test.a, test.b, res);
                break;
            case NAIVE:
                matr_mult_ellpack_naive(test.a, test.b, res);
                break;
            default:
                break;
        }
        if (!compare_ellpack(res, test.r)) {
            fprintf(report, "error on testcase: %d with matrices:\n", test_case);
            print_ellpack(report, test.a, "A");
            print_ellpack(report, test.b, "B");
            print_ellpack(report, test.r, "expected");
            print_ellpack(report, res, "but found");
            free_ellpack(test.a);
            free_ellpack(test.b);
            free_ellpack(test.r);
            free_ellpack(res);
            return;
        }
        free_ellpack(test.a);
        free_ellpack(test.b);
        free_ellpack(test.r);
        free_ellpack(res);
    }
    fprintf(report, "-- all tests passed --\n");
}
