CFLAGS=-O2 -fopenmp -pthread -Wall -Wextra -Werror
DEFINES=-DUSE_COMBINER -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) 
DEFINES_SPINLOCK=-DUSE_SPIN_LOCK

default: all

all: hashmin pagerank hashmin_spinlock pagerank_spinlock

hashmin:
	gcc -o hashmin hashmin.c -std=c99 $(CFLAGS) $(DEFINES)

pagerank:
	gcc -o pagerank pagerank.c -std=c99 $(CFLAGS) $(DEFINES)

hashmin_spinlock:
	gcc -o hashmin_spinlock hashmin.c -std=gnu99 $(CFLAGS) $(DEFINES) $(DEFINES_SPINLOCK)

pagerank_spinlock:
	gcc -o pagerank_spinlock pagerank.c -std=gnu99 $(CFLAGS) $(DEFINES) $(DEFINES_SPINLOCK)
