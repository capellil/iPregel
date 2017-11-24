CFLAGS=-O2 -fopenmp -pthread -Wall -Wextra -Werror -Wfatal-errors
DEFINES=-DOMP_NUM_THREADS=$(OMP_NUM_THREADS)
DEFINES_COMBINER=-DUSE_COMBINER -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) 
DEFINES_COMBINER_SPINLOCK=-DUSE_SPIN_LOCK

default: all

all: hashmin_combiner \
	 hashmin_combiner_spinlock \
	 pagerank_combiner \
	 pagerank_combiner_spinlock

hashmin:
	gcc -o hashmin hashmin.c -std=c99 $(DEFINES) $(CFLAGS)

pagerank:
	gcc -o pagerank pagerank.c -std=c99 $(DEFINES) $(CFLAGS)

hashmin_combiner:
	gcc -o hashmin_combiner hashmin.c -std=c99 $(DEFINES_COMBINER) $(CFLAGS)

pagerank_combiner:
	gcc -o pagerank_combiner pagerank.c -std=c99 $(DEFINES_COMBINER) $(CFLAGS)

hashmin_combiner_spinlock:
	gcc -o hashmin_combiner_spinlock hashmin.c -std=gnu99 $(DEFINES_COMBINER) $(DEFINES_COMBINER_SPINLOCK) $(CFLAGS)

pagerank_combiner_spinlock:
	gcc -o pagerank_combiner_spinlock pagerank.c -std=gnu99 $(DEFINES_COMBINER) $(DEFINES_COMBINER_SPINLOCK) $(CFLAGS)
