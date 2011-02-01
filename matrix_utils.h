#ifndef _MATRIX_UTILS
#define _MATRIX_UTILS

#include <stdlib.h>
#include <stdio.h>
#include <values.h>

#define elem_t double
#define MIN_ELEM DBL_MIN
#define msize_t int


/* Matrix */
struct matrix_t {
    elem_t * restrict * restrict data;
    msize_t n_rows;
    msize_t n_cols;
};


/* Represents matrix element with position */
struct elem_pos_t {
    msize_t row, col;
    elem_t value;
};

struct elem_pos_t* elem_pos(msize_t row, msize_t col, elem_t value);
void free_elem_pos(struct elem_pos_t* e);


/*
 * Returns maximal element of m1 * m2
 */
struct elem_pos_t* max_res_elem(struct matrix_t* m1, struct matrix_t* m2);

/*
 * Reads matrix from file
 */
struct matrix_t* read_matrix(FILE *f);


struct matrix_t* read_from_file(char* fname);
void write_to_file(struct elem_pos_t *res, char* fname);

/*
 * Deallocates matrix
 */
void free_matrix(struct matrix_t* m);

#ifndef NDEBUG
void print_matrix(struct matrix_t *m);
#endif

#endif /* _MATRIX_UTILS */

