CFLAGS=-O2 -fopenmp -pthread -Wall -Wextra -Werror -Wfatal-errors
DEFINES=-DOMP_NUM_THREADS=$(OMP_NUM_THREADS)
DEFINES_COMBINER=-DUSE_COMBINER -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) 
DEFINES_COMBINER_SPINLOCK=-DUSE_SPIN_LOCK
DEFINES_SINGLE_BROADCAST=-DSINGLE_BROADCAST
DEFINES_SPREAD=-DSPREAD

SRC_DIRECTORY=src
BENCHMARKS_DIRECTORY=benchmarks
BIN_DIRECTORY=bin

default: all

all: mirror_checker_combiner \
	 hashmin \
	 hashmin_combiner \
	 hashmin_combiner_spinlock \
	 hashmin_combiner_single_broadcast \
	 pagerank \
	 pagerank_combiner \
	 pagerank_combiner_spinlock \
	 pagerank_combiner_single_broadcast \
	 sssp \
	 sssp_combiner \
	 sssp_combiner_spinlock \
	 sssp_combiner_single_broadcast \
	 sssp_combiner_spread

mirror_checker_combiner:
	gcc -o $(BIN_DIRECTORY)/mirror_checker_combiner $(BENCHMARKS_DIRECTORY)/mirror_checker.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(CFLAGS)

hashmin:
	gcc -o $(BIN_DIRECTORY)/hashmin $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

hashmin_combiner:
	gcc -o $(BIN_DIRECTORY)/hashmin_combiner $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(CFLAGS)

hashmin_combiner_spinlock:
	gcc -o $(BIN_DIRECTORY)/hashmin_combiner_spinlock $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES_COMBINER) $(DEFINES_COMBINER_SPINLOCK) $(CFLAGS)

hashmin_combiner_single_broadcast:
	gcc -o $(BIN_DIRECTORY)/hashmin_combiner_single_broadcast $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

pagerank:
	gcc -o $(BIN_DIRECTORY)/pagerank $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

pagerank_combiner:
	gcc -o $(BIN_DIRECTORY)/pagerank_combiner $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(CFLAGS)

pagerank_combiner_spinlock:
	gcc -o $(BIN_DIRECTORY)/pagerank_combiner_spinlock $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES_COMBINER) $(DEFINES_COMBINER_SPINLOCK) $(CFLAGS)

pagerank_combiner_single_broadcast:
	gcc -o $(BIN_DIRECTORY)/pagerank_combiner_single_broadcast $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

sssp:
	gcc -o $(BIN_DIRECTORY)/sssp $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

sssp_combiner:
	gcc -o $(BIN_DIRECTORY)/sssp_combiner $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(CFLAGS)

sssp_combiner_spinlock:
	gcc -o $(BIN_DIRECTORY)/sssp_combiner_spinlock $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES_COMBINER) $(DEFINES_COMBINER_SPINLOCK) $(CFLAGS)

sssp_combiner_single_broadcast:
	gcc -o $(BIN_DIRECTORY)/sssp_combiner_single_broadcast $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

sssp_combiner_spread:
	gcc -o $(BIN_DIRECTORY)/sssp_combiner_spread $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES_COMBINER) $(DEFINES_SPREAD) $(CFLAGS)

clean:
	rm -rf $(BIN_DIRECTORY)
