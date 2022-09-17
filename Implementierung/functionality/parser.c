//
// Created by m0rtis on 7/24/22.
//

#include "parser.h"
#include "ellpack_utility.h"

#include <error.h>
#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define CSV_LINE_LENGTH 61

static void token_error(char* path, unsigned long long line_count) {
    error(1, 0, "Error while parsing matrix %s: Invalid line %llu: Token error", path, line_count);
}

static void parse_error(char* path, unsigned long long line_count, char* token) {
    error(1, 0, "Error while parsing matrix %s: Invalid line %llu: Parse error for %s", path, line_count, token);
}

static int skip_lines(FILE *file, unsigned long long num) {
    char line[20] = {0};
    for(unsigned long long run = 0; run < num; ++run) {
        if (!fgets(line, 20, file)) {
            return -1;
        }
    }
    return 0;
}

struct EllpackMatrix* parse_matrix(char *matrix_path) {
    FILE *matrix_file = fopen(matrix_path, "r");
    if(!matrix_file) {
        error(1, 0, "Error while opening matrix file %s, do you have the correct permissions?", matrix_path);
    }

    // Read the first 2 lines containing WIDTH\nHEIGHT in u_int64_t format
    char line[20] = {0};
    unsigned long long line_count = 0;
    char* end_ptr;
    long width = 0;
    long height = 0;

    while (fgets(line, 20, matrix_file)) {
        if(line_count > 1) {
            break;
        } else if (line_count == 0) {
            errno = 0;
            height = strtol(line, &end_ptr, 10);
            if (errno != 0 || *line == '\0' || *end_ptr != '\n') {
                error(1, 0, "Error while parsing matrix size definitions in %s at line %llu", matrix_path, line_count + 1);
            }
        } else if (line_count == 1) {
            errno = 0;
            width = strtol(line, &end_ptr, 10);
            if (errno != 0 || *line == '\0' || *end_ptr != '\n') {
                error(1, 0, "Error while parsing matrix size definitions in %s at line %llu", matrix_path, line_count + 1);
            }
        }
        ++line_count;
    }
    if(width <= 0 || height <= 0) {
        error(1, 0, "Matrix size may not be 0! At %s", matrix_path);
    }
    if(width > UINT_MAX || height > UINT_MAX) {
        error(1, 0, "Matrix size dimensions are too large! At %s", matrix_path);
    }


    // Scan matrix for shrinking possibilities
    // CSV separator is ; -> FLOAT;INT;INT so line length = (float) 18 + (int) 20 + (int) 20 + (;) 2 + (\n) 1 = 61
    printf("[SCAN] Scanning matrix %s ...\n", matrix_path);

    char matrix_line[CSV_LINE_LENGTH] = {0};
    char* token;

    line_count = 4;

    u_int64_t max_width = 0;
    u_int64_t current_row = 0;
    u_int64_t count_used = 0;

    while (fgets(matrix_line, CSV_LINE_LENGTH, matrix_file)) {
        token = strtok(matrix_line, ";");
        if (!token) {
            token_error(matrix_path, line_count);
        }
        errno = 0;
        long row_raw = strtol(token, &end_ptr, 10);
        if (errno != 0 || *token == '\0' || *end_ptr != '\0') {
            parse_error(matrix_path, line_count, token);
        }
        if(row_raw > UINT_MAX || row_raw < 0) {
            error(1, 0, "Error while scanning matrix %s, Invalid line %llu: Row number is out of bounds", matrix_path, line_count);
        }
        u_int64_t row = (u_int64_t) row_raw;

        if(row >= (u_int64_t) height) {
            error(1, 0, "Error while scanning matrix %s, Invalid line %llu: Row number is out of bounds", matrix_path, line_count);
        }

        if(row > current_row) {
            current_row = row;
            if(max_width < count_used) {
                max_width = count_used;
            }
            count_used = 0;
        }

        token = strtok(NULL, ";");
        if (!token) {
            token_error(matrix_path, line_count);
        }
        errno = 0;
        long column_raw = strtol(token, &end_ptr, 10);
        if (errno != 0 || *token == '\0' || *end_ptr != '\0') {
            parse_error(matrix_path, line_count, token);
        }
        if(column_raw > UINT_MAX || column_raw < 0) {
            error(1, 0, "Error while scanning matrix %s, Invalid line %llu: Column number is out of bounds", matrix_path, line_count);
        }
        u_int64_t column = (u_int64_t) column_raw;

        if(column >= (u_int64_t) width) {
            error(1, 0, "Error while scanning matrix %s, Invalid line %llu: Column number is out of bounds", matrix_path, line_count);
        }

        ++count_used;
        ++line_count;
    }

    if(max_width <= 0) {
        max_width = (u_int64_t) width;
    }

    printf("[SCAN] Completed, Shrinking matrix width from %lu -> %lu\n", width, max_width);
    printf("[INIT] Allocating %lu bytes of memory for matrix %s\n", (4 * max_width * height) + (8 * max_width * height), matrix_path);

    struct EllpackMatrix* matrix = make_ellpack((u_int64_t) width, (u_int64_t) height, max_width, matrix_path);

    for(u_int64_t run_row = 0; run_row < matrix->height; ++run_row) {
        for(u_int64_t run_col = 0; run_col < matrix->width; ++run_col) {
            matrix->indices[run_row * matrix->width + run_col] = 0;
            matrix->values[run_row * matrix->width + run_col] = 0;
        }
    }

    printf("[INIT] Reading data of matrix %s into memory\n", matrix_path);

    // Go to start of file (after height and width declarations)
    rewind(matrix_file);

    int skr = skip_lines(matrix_file, 3);
    if(skr != 0) {
        free_ellpack(matrix);
        error(1, 0, "I/O Error while skipping lines in %s", matrix_path);
    }

    // Read matrix in CSV format
    // CSV separator is ; -> FLOAT;INT;INT so line length = (float) 18 + (int) 20 + (int) 20 + (;) 2 + (\n) 1 = 61

    line_count = 4;

    while (fgets(matrix_line, CSV_LINE_LENGTH, matrix_file)) {
        token = strtok(matrix_line, ";");
        if (!token) {
            free_ellpack(matrix);
            token_error(matrix_path, line_count);
        }
        errno = 0;
        long row_raw = strtol(token, &end_ptr, 10);
        if (errno != 0 || *token == '\0' || *end_ptr != '\0') {
            parse_error(matrix_path, line_count, token);
        }
        if(row_raw > UINT_MAX || row_raw < 0) {
            free_ellpack(matrix);
            error(1, 0, "Error while parsing matrix %s, Invalid line %llu: Row number is out of bounds", matrix_path, line_count);
        }
        u_int64_t row = (u_int64_t) row_raw;

        if(row >= matrix->height) {
            free_ellpack(matrix);
            error(1, 0, "Error while parsing matrix %s, Invalid line %llu: Row number is out of bounds", matrix_path, line_count);
        }

        token = strtok(NULL, ";");
        if (!token) {
            free_ellpack(matrix);
            token_error(matrix_path, line_count);
        }
        errno = 0;
        long column_raw = strtol(token, &end_ptr, 10);
        if (errno != 0 || *token == '\0' || *end_ptr != '\0') {
            free_ellpack(matrix);
            parse_error(matrix_path, line_count, token);
        }
        if(column_raw > UINT_MAX || column_raw < 0) {
            free_ellpack(matrix);
            error(1, 0, "Error while parsing matrix %s, Invalid line %llu: Column number is out of bounds", matrix_path, line_count);
        }
        u_int64_t column = (u_int64_t) column_raw;

        if(column >= matrix->real_width) {
            free_ellpack(matrix);
            error(1, 0, "Error while parsing matrix %s, Invalid line %llu: Column number is out of bounds", matrix_path, line_count);
        }

        token = strtok(NULL, ";");
        if (!token) {
            free_ellpack(matrix);
            token_error(matrix_path, line_count);
        }
        errno = 0;
        float value = strtof(token, &end_ptr);
        if (errno != 0 || *token == '\0' || *end_ptr != '\n') {
            free_ellpack(matrix);
            parse_error(matrix_path, line_count, token);
        }

        u_int64_t new_col = 0;
        bool col_found = false;

        for (u_int64_t r_col = 0; r_col < matrix->width; ++r_col) {
            if (matrix->values[row * matrix->width + r_col] == 0) {
                new_col = r_col;
                col_found = true;
                break;
            }
        }

        if(!col_found) {
            free_ellpack(matrix);
            error(1, 0, "Error while parsing matrix %s, size overflowed at line %llu", matrix_path, line_count);
        }

        matrix->values[row * matrix->width + new_col] = value;
        matrix->indices[row * matrix->width + new_col] = column;

        ++line_count;
    }

    fclose(matrix_file);
    return matrix;
}

void write_matrix(struct EllpackMatrix* matrix, char *out_path) {
    FILE *out_file = fopen(out_path, "w");
    if(!out_file) {
        free_ellpack(matrix);
        error(1, 0, "Error while opening matrix file %s, do you have the correct permissions?", out_path);
    }

    // Write the first 2 lines containing WIDTH\nHEIGHT in u_int64_t format
    fprintf(out_file, "%lu\n", matrix->height);
    fprintf(out_file, "%lu\n", matrix->real_width);
    fprintf(out_file, "\n");

    // Print the matrix in reduced coordinate schema
    for(u_int64_t run_row = 0; run_row < matrix->height; ++run_row) {
        for(u_int64_t run_col = 0; run_col < matrix->width; ++run_col) {
            u_int64_t index_entry = matrix->indices[run_row * matrix->width + run_col];
            float value_entry = matrix->values[run_row * matrix->width + run_col];
            if(value_entry != 0) {
                u_int64_t real_column = index_entry;
                fprintf(out_file, "%lu;%lu;%.9e", run_row, real_column, value_entry);
                if(run_row < (matrix->height - 1) || run_col < (matrix->width - 1)) {
                    fprintf(out_file, "\n");
                }
            }
        }
    }

    if (ferror(out_file)) {
        fclose(out_file);
        free_ellpack(matrix);
        error(1, 0, "Error while writing matrix file %s", out_path);
    }
    fclose(out_file);
}
