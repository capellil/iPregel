CFLAGS=-std=c99 -O2 -fopenmp -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) -pthread -DUSE_COMBINER -Wall -Wextra -Werror

default: all

all: hashmin pagerank

hashmin:
	gcc -o hashmin hashmin.c $(CFLAGS)

pagerank:
	gcc -o pagerank pagerank.c $(CFLAGS)
