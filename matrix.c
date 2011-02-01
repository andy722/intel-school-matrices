#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <assert.h>

#include "matrix_utils.h"


int main(int argc, char** argv) {

    struct matrix_t* m1;
    struct matrix_t* m2;
    struct elem_pos_t* max_elem;

    assert (argc == 4);

    /* Read matrices */
    m1 = read_from_file(argv[1]);
    m2 = read_from_file(argv[2]);

    /* Crunch the numbers */
    max_elem = max_res_elem(m1, m2);

    /* Write result */
    write_to_file(max_elem, argv[3]);

    free_matrix(m1);
    free_matrix(m2);
    free_elem_pos(max_elem);

    return EXIT_SUCCESS;
}

