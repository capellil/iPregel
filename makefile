CFLAGS=-std=gnu99 -O2 -fopenmp -pthread -Wall -Wextra -Werror 
DEFINES=-DUSE_COMBINER -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) -DUSE_SPIN_LOCK

default: all

all: hashmin pagerank

hashmin:
	gcc -o hashmin hashmin.c $(CFLAGS) $(DEFINES)

pagerank:
	gcc -o pagerank pagerank.c $(CFLAGS) $(DEFINES)
