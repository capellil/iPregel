default: all

all:
	gcc -o main main.c -O2 -fopenmp -DOMP_NUM_THREADS=$(OMP_NUM_THREADS)
