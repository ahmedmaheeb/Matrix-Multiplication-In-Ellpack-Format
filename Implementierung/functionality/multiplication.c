#include "multiplication.h"
#include <string.h>
#include "ellpack_utility.h"

void matr_mult_ellpack(const void* a, const void* b, void* result) {
    struct EllpackMatrix *r = (struct EllpackMatrix *) result;
    if (!valid_ellpack(a) || !valid_ellpack(b)) {
        error(1, 0, "an argument matrix has wrong format");
        return;
    }
    // b is transposed to bx and for each entry in row i column j the result is
    // the product of row i in a and row j in bx
    // go through the two rows and find equal indices => merge
    struct EllpackMatrix *ax = (struct EllpackMatrix *) a;
    struct EllpackMatrix *bx = transpose_ellpack((struct EllpackMatrix *) b);
    if (!bx) {
        error(1, 0, "transpose failed");
        return;
    }
    r->height = ax->height;
    // arrays of result rows
    float **r_values = calloc(r->height, sizeof(float *));
    u_int64_t **r_indices = calloc(r->height, sizeof(u_int64_t *));
    u_int64_t *r_row_lengths = calloc(ax->height, sizeof(u_int64_t));
    u_int64_t max_width = 0;
    // array of upper limit size for each result row
    float *r_row_values = calloc(bx->height, sizeof(float));
    u_int64_t *r_row_indices = calloc(bx->height, sizeof(u_int64_t));
    if (!r_values || !r_indices || !r_row_lengths || !r_row_indices || !r_row_values) {
        r->height = 0; // skip all loops and go to cleanup
    }
    for (u_int64_t r_row_i = 0; r_row_i < r->height; r_row_i++) { // Zeileniteration
        u_int64_t r_column_counter = 0; // only not null results are written to result row, to mantain ellpack form
        for (u_int64_t b_row_i = 0; b_row_i < bx->height; b_row_i++) {
            // indices of currently merged entries, iterate through the row of a and b
            u_int64_t a_column_i = 0;
            u_int64_t b_column_i = 0;
            float res_sum = 0.0F;
            // check if merging ended
            while (a_column_i < ax->width && b_column_i < bx->width) {
                // the indices are equal and are to be multiplied, otherwise the lower one increments
                if (ax->indices[r_row_i * ax->width + a_column_i] == bx->indices[b_row_i * bx->width + b_column_i]) {
                    res_sum += ax->values[r_row_i * ax->width + a_column_i] * bx->values[b_row_i * bx->width + b_column_i];
                    a_column_i++;
                    b_column_i++;
                } else if (ax->indices[r_row_i * ax->width + a_column_i] > bx->indices[b_row_i * bx->width + b_column_i]) {
                    b_column_i++;
                } else {
                    a_column_i++;
                }
            }
            // only add the result entry if it s not zero
            if (res_sum != 0.0) {
                r_row_values[r_column_counter] = res_sum;
                r_row_indices[r_column_counter] = b_row_i;
                r_column_counter++;
            }
        }
        if (r_column_counter > max_width) {
            max_width = r_column_counter;
        }
        // store the resulting row with its length
        r_values[r_row_i] = calloc(r_column_counter, sizeof(float));
        r_indices[r_row_i] = calloc(r_column_counter, sizeof(u_int64_t));
        if (!r_values[r_row_i] || !r_indices[r_row_i]) {
            r->height = r_row_i + 1; // only clean up to this row in the flatten method
            break;
        }
        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(u_int64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }
    r->width = max_width;
    flatten_ellpack(r, r_values, r_indices, r_row_lengths);
    free(r_row_values);
    free(r_row_indices);
    free_ellpack(bx);
    r->real_width = ((struct EllpackMatrix *) b)->real_width;
}

void matr_mult_ellpack_vectorised(const void* a, const void* b, void* result) {
    struct EllpackMatrix *r = (struct EllpackMatrix *) result;
    if (!valid_ellpack(a) || !valid_ellpack(b)) {
        error(1, 0, "an argument matrix has wrong format");
        return;
    }
    // b is transposed to bx and for each entry in row i column j the result is
    // the product of row i in a and row j in bx
    // go through the two rows and find equal indices => merge
    struct EllpackMatrix *ax = (struct EllpackMatrix *) a;
    struct EllpackMatrix *bx = transpose_ellpack((struct EllpackMatrix *) b);
    if (!bx) {
        error(1, 0, "transpose failed");
        return;
    }
    r->height = ax->height;
    // arrays of result rows
    float **r_values = calloc(r->height, sizeof(float *));
    u_int64_t **r_indices = calloc(r->height, sizeof(u_int64_t *));
    u_int64_t *r_row_lengths = calloc(ax->height, sizeof(u_int64_t));
    u_int64_t max_width = 0;
    // array of upper limit size for each result row
    float *r_row_values = calloc(bx->height, sizeof(float));
    u_int64_t *r_row_indices = calloc(bx->height, sizeof(u_int64_t));
    if (!r_values || !r_indices || !r_row_lengths || !r_row_indices || !r_row_values) {
        r->height = 0; // skip all loops and go to cleanup
    }
    for (u_int64_t r_row_i = 0; r_row_i < r->height; r_row_i++) { // Zeileniteration
        u_int64_t r_column_counter = 0; // only not null results are written to result row, to mantain ellpack form
        for (u_int64_t b_row_i = 0; b_row_i < bx->height; b_row_i++) {
            // indices of currently merged entries, iterate through the row of a and b
            u_int64_t a_column_i = 0;
            u_int64_t b_column_i = 0;
            __m128 a_vec_val;
            __m128 b_vec_val;
            __m128 res_vec = _mm_setzero_ps();
            int cnt_vals = 0;
            float a_temp[4], b_temp[4];
            // check if merging ended
            while (a_column_i < ax->width && b_column_i < bx->width) {
                if (ax->indices[r_row_i * ax->width + a_column_i] == bx->indices[b_row_i * bx->width + b_column_i]) {
                    a_temp[cnt_vals] = ax->values[r_row_i * ax->width + a_column_i];
                    b_temp[cnt_vals] = bx->values[b_row_i * bx->width + b_column_i];
                    cnt_vals++;
                    a_column_i++;
                    b_column_i++;
                    // für gleiche indices wird multipliziert
                } else if (ax->indices[r_row_i * ax->width + a_column_i] > bx->indices[b_row_i * bx->width + b_column_i]) {
                    b_column_i++;
                } else {
                    a_column_i++;
                }
                if (cnt_vals==4) {
                    a_vec_val = _mm_load_ss(a_temp);
                    b_vec_val = _mm_load_ss(b_temp);
                    a_vec_val = _mm_mul_ps(a_vec_val,b_vec_val);
                    res_vec = _mm_add_ps(a_vec_val, res_vec);
                    cnt_vals = 0;
                }
                // , sonst wird der kleinere index incrementiert
            } // "merge-multiplication" Zeile mal Spalte (transponierte Zeile)
            float res_sum = hsum_ps_sse1(res_vec);
            for(int i=0; i<cnt_vals;i++){
                res_sum += a_temp[i]*b_temp[i];
            }
            // only add the result entry if it s not zero
            if (res_sum != 0.0) {
                r_row_values[r_column_counter] = res_sum;
                r_row_indices[r_column_counter] = b_row_i;
                r_column_counter++;
            }
        }
        if (r_column_counter > max_width) {
            max_width = r_column_counter;
        }
        // store the resulting row with its length
        r_values[r_row_i] = calloc(r_column_counter, sizeof(float));
        r_indices[r_row_i] = calloc(r_column_counter, sizeof(u_int64_t));
        if (!r_values[r_row_i] || !r_indices[r_row_i]) {
            r->height = r_row_i + 1; // only clean up to this row in the flatten method
            break;
        }
        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(u_int64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }
    r->width = max_width;
    flatten_ellpack(r, r_values, r_indices, r_row_lengths);
    free(r_row_values);
    free(r_row_indices);
    free_ellpack(bx);
    r->real_width = ((struct EllpackMatrix *) b)->real_width;
}

void matr_mult_ellpack_naive(const void* a, const void* b, void* result) {
    struct EllpackMatrix *r = (struct EllpackMatrix *) result;
    if (!valid_ellpack(a) || !valid_ellpack(b)) {
        error(1, 0, "an argument matrix has wrong format");
        return;
    }
    struct EllpackMatrix *ax = (struct EllpackMatrix *) a;
    struct EllpackMatrix *bx = (struct EllpackMatrix *) b;
    r->height = ax->height;
    // arrays of result rows
    float **r_values = calloc(r->height, sizeof(float *));
    u_int64_t **r_indices = calloc(r->height, sizeof(u_int64_t *));
    u_int64_t *r_row_lengths = calloc(ax->height, sizeof(u_int64_t));
    u_int64_t max_width = 0;
    // array of upper limit size for each result row
    float *r_row_values = calloc(bx->height, sizeof(float));
    u_int64_t *r_row_indices = calloc(bx->height, sizeof(u_int64_t));
    if (!r_values || !r_indices || !r_row_lengths || !r_row_indices || !r_row_values) {
        r->height = 0; // skip all loops and go to cleanup
    }
    for (u_int64_t r_row_i = 0; r_row_i < r->height; ++r_row_i) { // iter: i
        u_int64_t r_column_counter = 0; // only not null results are written to result row, to mantain ellpack form
        for (u_int64_t b_col_i = 0; b_col_i < bx->real_width; ++b_col_i) { // iter: j
            float res_sum = 0.0F;
            for (u_int64_t a_column_i = 0; a_column_i < ax->width; ++a_column_i) { // iter: k
                u_int64_t b_line_to_search = ax->indices[r_row_i * ax->width + a_column_i];
                u_int64_t b_search_result = bx->width;
                for (u_int64_t b_search_col_i = 0; b_search_col_i < bx->width; ++b_search_col_i) {
                    if (bx->indices[b_line_to_search * bx->width + b_search_col_i] == b_col_i) {
                        b_search_result = b_search_col_i;
                        break;
                    }
                }
                if (b_search_result < bx->width) {
                    res_sum += ax->values[r_row_i * ax->width + a_column_i] * bx->values[b_line_to_search * bx->width + b_search_result];
                }
            }
            // only add the result entry if it s not zero
            if (res_sum != 0.0) {
                r_row_values[r_column_counter] = res_sum;
                r_row_indices[r_column_counter] = b_col_i;
                r_column_counter++;
            }
        }
        if (r_column_counter > max_width) {
            max_width = r_column_counter;
        }
        // store the resulting row with its length
        r_values[r_row_i] = calloc(r_column_counter, sizeof(float));
        r_indices[r_row_i] = calloc(r_column_counter, sizeof(u_int64_t));
        if (!r_values[r_row_i] || !r_indices[r_row_i]) {
            r->height = r_row_i + 1; // only clean up to this row in the flatten method
            break;
        }
        memcpy(r_values[r_row_i], r_row_values, sizeof(float) * r_column_counter);
        memcpy(r_indices[r_row_i], r_row_indices, sizeof(u_int64_t) * r_column_counter);
        r_row_lengths[r_row_i] = r_column_counter;
    }
    r->width = max_width;
    flatten_ellpack(r, r_values, r_indices, r_row_lengths);
    free(r_row_values);
    free(r_row_indices);
    r->real_width = ((struct EllpackMatrix *) b)->real_width;
}
