//
// Created by m0rtis on 6/27/22.
//
#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <error.h>
#include <stdbool.h>

#include "functionality/ellpack_utility.h"
#include "functionality/multiplication.h"
#include "functionality/testing.h"
#include "functionality/benchmarking.h"
#include "functionality/parser.h"

const char *argp_program_version = "ELLMUL version v0.1.0-dev";
static char doc[] = "ellmul: fast multiplication of ellpack matrices";
__attribute__((unused)) const char *argp_program_bug_address = "p.assmann@tum.de";

static char args_doc[] = "";

static struct argp_option options[] = {
        {"verbose", 'v', 0, 0, "Produce verbose output", 3},
        {"help", 'h', 0, 0, "Give this help list", 3},
        {"impl", 'V', "int", 0, "Which implementation to run", 2},
        {"benchmark", 'B', "int", OPTION_ARG_OPTIONAL, "Benchmark with iterations", 2},
        {"test", 'T', "int", 0, "Test an implementation", 2},
        {"amatrix", 'a', "file", 0, "Path to input Matrix A", 1},
        {"bmatrix", 'b', "file", 0, "Path to input Matrix B", 1},
        {"output", 'o', "file", 0, "Path to output Matrix", 1},
        {0}
};

struct arguments {
    int verbose, version, benchmark, test, help;
    char *amatrix;
    char *bmatrix;
    char *output;
};

static const int MAX_IMPL = 3;

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    char* end_ptr;

    switch (key) {
        case 'h':
            argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
            exit(0);
        case 'v':
            arguments->verbose = 1;
            break;
        case 'V':
            ;
            errno = 0;
            int version = (int) strtol(arg, &end_ptr, 10);
            if (errno != 0 || *arg == '\0' || *end_ptr != '\0') {
                argp_failure(state, 1, 0, "not a valid implementation: %s", arg);
            }
            if (version < 0 || version >= MAX_IMPL) {
                argp_failure(state, 1, 0, "not a valid implementation: %s", arg);
            }
            arguments->version = version;
            break;
        case 'B':
            ;
            errno = 0;
            int benchmark;
            if (arg) {
                benchmark = (int) strtol(arg, &end_ptr, 10);
            } else {
                benchmark = 10;
                arguments->benchmark = benchmark;
                break;
            }
            if (errno != 0 || *arg == '\0' || *end_ptr != '\0') {
                argp_failure(state, 1, 0, "not a valid benchmark count: %s", arg);
            }
            if (benchmark <= 0 || benchmark > 100000) {
                argp_failure(state, 1, 0, "not a valid benchmark count (out of bounds): %s", arg);
            }
            arguments->benchmark = benchmark;
            break;
        case 'T':
            ;
            errno = 0;
            int test = (int) strtol(arg, &end_ptr, 10);
            if (errno != 0 || *arg == '\0' || *end_ptr != '\0') {
                argp_failure(state, 1, 0, "not a valid testing target: %s", arg);
            }
            if (test < 0 || test >= MAX_IMPL) {
                argp_failure(state, 1, 0, "not a valid testing target: %s", arg);
            }
            arguments->test = test;
            break;
        case 'a':
            ;
            if (access(arg, R_OK) == 0) {
                arguments->amatrix = arg;
            } else {
                argp_failure(state, 1, 0, "amatrix file does not exist or missing read permission: %s", arg);
            }
            break;
        case 'b':
            ;
            if (access(arg, R_OK) == 0) {
                arguments->bmatrix = arg;
            } else {
                argp_failure(state, 1, 0, "bmatrix file does not exist or missing read permission: %s", arg);
            }
            break;
        case 'o':
            ;
            fopen(arg, "w");
            if (access(arg, W_OK) == 0) {
                arguments->output = arg;
            } else {
                argp_failure(state, 1, 0, "output file does not exist or missing write permission: %s", arg);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};


void dump_inputs(struct EllpackMatrix* amatrix, struct EllpackMatrix* bmatrix) {
    printf("\n");
    print_ellpack(stdout, amatrix, "A");
    printf("\n");
    print_ellpack(stdout, bmatrix, "B");
    printf("\n");
}

int main (int argc, char** argv) {
    struct arguments arguments;
    arguments.verbose = 0;
    arguments.amatrix = "a.mat";
    arguments.bmatrix = "b.mat";
    arguments.output = "out.mat";
    arguments.version = 0;
    arguments.benchmark = -1;
    arguments.test = -1;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if(arguments.test != -1) {
        switch (arguments.test) {
            case 0:
                testing(LINEAR, stdout);
                break;
            case 1:
                testing(VECTORIZED, stdout);
                break;
            case 2:
                testing(NAIVE, stdout);
                break;
        }
        return 0;
    }

    printf("%s\n\n", argp_program_version);

    printf("[LOAD] Loading Matrix A ...\n");
    struct EllpackMatrix* amatrix = parse_matrix(arguments.amatrix);
    printf("[DONE] Matrix A loaded, Dimensions: [%lu (formerly %lu) x %lu]\n\n", amatrix->width, amatrix->real_width, amatrix->height);

    printf("[LOAD] Loading Matrix B ...\n");
    struct EllpackMatrix* bmatrix = parse_matrix(arguments.bmatrix);

    if(amatrix->height != bmatrix->real_width) {
        free_all((struct EllpackMatrix *[]){amatrix, bmatrix}, 2);
        error(1, 0, "Error: Dimensions mismatch: Matrix A (height) must equal Matrix B (width) for multiplication. %lu != %lu", amatrix->height, bmatrix->real_width);
    }

    printf("[DONE] Matrix B loaded, Dimensions: [%lu (formerly %lu) x %lu]\n\n", bmatrix->width, bmatrix->real_width, bmatrix->height);
    printf("[LOAD_COMPLETE] Ready for multiplication\n");
    printf("\n[MUL] Multiplication in progress ...\n");

    struct EllpackMatrix* result = calloc(1, sizeof(*result));
    if(arguments.benchmark != -1) {
        benchmark(arguments.version, arguments.benchmark, amatrix, bmatrix, result);
    } else {
        switch (arguments.version) {
            case 0:
                matr_mult_ellpack(amatrix, bmatrix, result);
                break;
            case 1:
                matr_mult_ellpack_vectorised(amatrix, bmatrix, result);
                break;
            case 2:
                matr_mult_ellpack_naive(amatrix, bmatrix, result);
                break;
        }
    }

    printf("\n[SAVE] Writing result matrix %s\n", arguments.output);
    write_matrix(result, arguments.output);
    printf("[FREE] Freeing used memory ...\n");
    free_all((struct EllpackMatrix *[]){amatrix, bmatrix, result}, 3);
}
