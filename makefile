default: all

all:
	gcc -o main main.c -O2 -std=c99 -fopenmp -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) -pg -DUSE_COMBINER -pthread
