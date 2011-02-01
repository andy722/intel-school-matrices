#!

CC=icc
OUT=matrix

OPTIMS="-fast -restrict"
OPTS="$OPTIMS -strict-ansi -w0 -DNDEBUG"

rm -f $OUT

$CC $OPTS matrix.c matrix_utils.c -o $OUT
