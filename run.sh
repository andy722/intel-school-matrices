
TEST=4

time ./matrix tests/$TEST-m1 tests/$TEST-m2 res4 >log4 && cat res4
#gdb --args ./matrix tests/$TEST-m1 tests/$TEST-m2 res 
