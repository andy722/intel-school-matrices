#!/bin/bash

NAME=${1:-"test"}

N=1000
M=2000

M1=$NAME-m1
M2=$NAME-m2

echo -n "$N $M " >$M1
echo -n "$M $N " >$M2

fill() {
    i=0
    ELEMS=$[$N * $M]
    while [ $i -lt $ELEMS ]; do
        echo -n "$RANDOM " >>$1
        i=$[$i + 1]
    done
}

fill $M1
fill $M2

