#include <assert.h>
#include <err.h>

#include "matrix_utils.h"

#define ELEM_ALIGN    32
#define PTR_ALIGN     32

static elem_t mult_vect(elem_t* v1, elem_t* v2, msize_t size) {
    elem_t res = 0;
    msize_t n;
    
    #pragma vector aligned
    for (n = 0; n < size; n++) {
        res += v1[n] * v2[n];
    }

    return res;
}

static struct matrix_t* alloc_matrix(msize_t n_rows, msize_t n_cols) {
    struct matrix_t* m;
    elem_t** data;

    m = (struct matrix_t*) _mm_malloc(sizeof(struct matrix_t), PTR_ALIGN);
    if (m == NULL) {
	return NULL;
    }

    if ((data = (elem_t**) _mm_malloc(sizeof(elem_t *) * n_rows, PTR_ALIGN)) == NULL) {
	return NULL;
    }

    msize_t n;
    for (n = 0; n < n_rows; n++) {
	if ((data[n] = (elem_t *) _mm_malloc(sizeof(elem_t) * n_cols, ELEM_ALIGN)) == NULL) {
	    return NULL;
	}
    }
   
    m->data = data;
    m->n_rows = n_rows;
    m->n_cols = n_cols;

    return m;
}


void free_matrix(struct matrix_t* m) {
    msize_t n;
    for (n = 0; n < m->n_rows; n++) {
        _mm_free(m->data[n]);
    }

    _mm_free((void *)m->data);
    _mm_free(m);
}

static struct matrix_t* transpose(struct matrix_t* m) {    
    struct matrix_t* t;

    if ((t = alloc_matrix(m->n_cols, m->n_rows)) == NULL) {
	return NULL;
    }

    /* Copying may look sloow, but it's not the bottleneck */
    {
        msize_t row, col;

	elem_t * restrict * restrict src;
	elem_t * restrict * restrict dst;
	
	src = m->data;
	dst = t->data;
        for (row = 0; row < m->n_rows; row++) {
            for (col = 0; col < m->n_cols; col++) {
                dst[col][row] = src[row][col];
            }
        }
    }

    return t;
}


/*
 * Returns maximal element of m1 * m2.
 */
struct elem_pos_t* max_res_elem(struct matrix_t* m1, struct matrix_t* m2) {
    msize_t row, col;
    elem_t* restrict res;

    msize_t max_row, max_col;
    elem_t max_res;

#ifndef NDEBUG
    max_row = max_col = -1;
#endif
    max_res = MIN_ELEM;

#ifndef NDEBUG
    print_matrix(m1);
    print_matrix(m2);
#endif

    assert (m1->n_cols == m2->n_rows);

    /* Optimize for cache misses */
    if ((m2 = transpose(m2)) == NULL) {
	return NULL;
    }

    if ((res = (elem_t*) _mm_malloc(sizeof(elem_t) * m1->n_rows * m2->n_rows, ELEM_ALIGN)) == NULL) {
	return NULL;
    }

    {
        msize_t m1_nrows, m2_nrows, m1_ncols;
        m1_nrows = m1->n_rows;
        m2_nrows = m2->n_rows;
        m1_ncols = m1->n_cols;

        /* no dependencies between iterations */
        #pragma parallel
        for (row = 0; row < m1_nrows; row++) {
            #pragma parallel
            for (col = 0; col < m2_nrows; col++) {
                res[row * m2_nrows + col] = 
                            mult_vect(m1->data[row], m2->data[col], m1_ncols);
            }
        }
    

	for (row = 0; row < m1_nrows; row++) {
            for (col = 0; col < m2_nrows; col++) {
		if (res[row * m2_nrows + col] > max_res) {
	            max_res = res[row * m2_nrows + col];
		    max_row = row;
		    max_col = col;
    	        }
            }
        }
    }
    free_matrix(m2);
    _mm_free(res);

    assert ((max_row >= 0) && (max_col >= 0));
    return elem_pos(max_row, max_col, max_res);
}


struct elem_pos_t* elem_pos(msize_t row, msize_t col, elem_t value) {
    struct elem_pos_t* res = 
        (struct elem_pos_t*) malloc(sizeof(struct elem_pos_t));

    if (res == NULL) {
        return NULL;
    }

    res->row = row;
    res->col = col;
    res->value = value;

    return res;
}


void free_elem_pos(struct elem_pos_t* e) {
    free(e);
}


/*
 * Reads matrix from file
 */
struct matrix_t* read_matrix(FILE *f) {
    struct matrix_t* matrix;
    msize_t n_rows, n_cols;
    
    assert (f != NULL);

    fscanf(f, "%d %d ", &n_rows, &n_cols); /* XXX */
    assert ((n_rows >= 0) && (n_cols >= 0));

    if ((matrix = alloc_matrix(n_rows, n_cols)) == NULL) {
	return NULL;
    }

    /* Read matrix elements */
    {
	msize_t row, col;
        elem_t * restrict * restrict data = matrix->data;

        for (row = 0; row < n_rows; row++) {
            for (col = 0; col < n_cols; col++) {
                fscanf(f, "%lf", &data[row][col]);  /* XXX */
            }
        }
    }

    return matrix;
}


struct matrix_t* read_from_file(char* fname) {
    struct matrix_t *res;

    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        err(EXIT_FAILURE, "%s", fname);
    }

    if ((res = read_matrix(file)) == NULL) {
        err(EXIT_FAILURE, "Error reading matrix: %s", fname);
    }

    if (fclose(file) != 0) {
        err(EXIT_FAILURE, "%s", fname);
    }

    return res;
}


void write_to_file(struct elem_pos_t *res, char* fname) {
    FILE *file = fopen(fname, "w");
    if (file == NULL) {
        err(EXIT_FAILURE, "%s", fname);
    }

    /* XXX: format may differ */
    fprintf(file, "%f %d %d", res->value, res->row, res->col);

    if (fclose(file) != 0) {
        err(EXIT_FAILURE, "%s", fname);
    }
}


#ifndef NDEBUG
void print_matrix(struct matrix_t *m) {
    msize_t row, col;

    printf("rows = %d, cols = %d\n", m->n_rows, m->n_cols);

    for (row = 0; row < m->n_rows; row++) {
        for (col = 0; col < m->n_cols; col++) {
            printf("%f ", m->data[row][col]);
        }
        printf("\n");
    }

}
#endif /* NDEBUG */
