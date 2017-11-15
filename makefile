default: all

all: hashmin pagerank

hashmin:
	gcc -o hashmin hashmin.c -O2 -std=c99 -fopenmp -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) -pthread -DUSE_COMBINER

pagerank:
	gcc -o pagerank pagerank.c -O2 -std=c99 -fopenmp -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) -pthread -DUSE_COMBINER
