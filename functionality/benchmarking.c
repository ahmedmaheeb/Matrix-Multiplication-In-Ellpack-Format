//
// Created by abdelgha on 7/15/22.
//

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include "benchmarking.h"
#include "ellpack_utility.h"
#include "multiplication.h"
#include "unistd.h"

double benchmark_once(int version, struct EllpackMatrix * a, struct EllpackMatrix * b, struct EllpackMatrix *res) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    switch (version) {
        case 0:
            matr_mult_ellpack(a, b, res);
            break;
        case 1:
            matr_mult_ellpack_vectorised(a, b, res);
            break;
        case 2:
            matr_mult_ellpack_naive(a, b, res);
            break;
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    return time;
}

void benchmark(int version, int iterations, struct EllpackMatrix *a, struct EllpackMatrix *b, struct EllpackMatrix *res) {
    printf("[BENCHMARK] Implementation %i with %i Iterations\n", version, iterations);
    double times[iterations];
    memset(times, 0, iterations * sizeof(double));
    for (int i = 0; i < iterations; ++i) {
        struct EllpackMatrix* result = malloc(sizeof(*result));
        double ms_result = benchmark_once(version, a, b, result);
        times[i] = ms_result;
        free_ellpack(result);
        int to_do = iterations - i;
        double eta = to_do + (to_do * ms_result);
        printf("[BENCHMARK] Implementation %i: Iteration %i / %i: %f (ETA: %f secs)\n", version, i + 1, iterations, ms_result, eta);
        sleep(1);
    }
    double sum = 0;
    double max = 0;
    double min = DBL_MAX;
    for (int i = 0; i < iterations; ++i) {
        sum = sum + times[i];
        if (times[i] < min) {
            min = times[i];
        }
        if (times[i] > max) {
            max = times[i];
        }
    }
    double avg = sum / iterations;

    printf("\n[RESULT] Benchmark results for %i:\n", version);
    printf("AVERAGE : %f\n", avg);
    printf("MAX : %f\n", max);
    printf("MIN : %f\n", min);
    matr_mult_ellpack(a, b, res);
}
