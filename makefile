CFLAGS=-O2 -fopenmp -pthread -Wall -Wextra -Werror -Wfatal-errors
DEFINES=-DOMP_NUM_THREADS=$(OMP_NUM_THREADS)
DEFINES_COMBINER=-DUSE_COMBINER -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) 
DEFINES_COMBINER_SPINLOCK=-DUSE_SPIN_LOCK
DEFINES_SINGLE_BROADCAST=-DSINGLE_BROADCAST

default: all

all: mirror_combiner \
	 hashmin \
	 hashmin_combiner \
	 hashmin_combiner_spinlock \
	 hashmin_combiner_single_broadcast \
	 pagerank \
	 pagerank_combiner \
	 pagerank_combiner_spinlock \
	 pagerank_combiner_single_broadcast

mirror_combiner:
	gcc -o mirror_combiner mirror.c -std=c99 $(DEFINES_COMBINER) $(CFLAGS)

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

hashmin_combiner_single_broadcast:
	gcc -o hashmin_combiner_single_broadcast hashmin.c -std=c99 $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

pagerank_combiner_single_broadcast:
	gcc -o pagerank_combiner_single_broadcast pagerank.c -std=c99 $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

clean:
	rm hashmin_combiner hashmin_combiner_spinlock hashmin_combiner_single_broadcast pagerank_combiner pagerank_combiner_spinlock pagerank_combiner_single_broadcast
