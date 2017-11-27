CFLAGS=-O2 -fopenmp -pthread -Wall -Wextra -Werror -Wfatal-errors -pg
DEFINES=-DOMP_NUM_THREADS=$(OMP_NUM_THREADS)
DEFINES_COMBINER=-DUSE_COMBINER
DEFINES_MUTEX=-DUSE_MUTEX
DEFINES_SPINLOCK=-DUSE_SPINLOCK
DEFINES_SPREAD=-DUSE_SPREAD
DEFINES_SINGLE_BROADCAST=-DUSE_SINGLE_BROADCAST

SUFFIX_NO_COMBINER=_no_combiner
SUFFIX_COMBINER=_combiner
SUFFIX_MUTEX=_mutex
SUFFIX_SPINLOCK=_spinlock
SUFFIX_SINGLE_BROADCAST=_single_broadcast
SUFFIX_SPREAD=_spread

SRC_DIRECTORY=src
BENCHMARKS_DIRECTORY=benchmarks
BIN_DIRECTORY=bin

default: all

all: graph_converter \
	 mirror_checker_combiner \
	 all_hashmin \
	 all_pagerank \
	 all_sssp

graph_converter:
	g++ -o $(BIN_DIRECTORY)/graph_converter $(SRC_DIRECTORY)/graph_converter.cpp -O2

mirror_checker_combiner:
	gcc -o $(BIN_DIRECTORY)/mirror_checker_combiner $(BENCHMARKS_DIRECTORY)/mirror_checker.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(CFLAGS)

###########
# HASHMIN #
###########
all_hashmin: hashmin$(SUFFIX_NO_COMBINER) \
			 hashmin$(SUFFIX_COMBINER)$(SUFFIX_MUTEX) \
			 hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK) \
			 hashmin$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST) \
			 hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST) \
			 hashmin$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD) \
			 hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)

hashmin$(SUFFIX_NO_COMBINER):
	gcc -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_NO_COMBINER) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

hashmin$(SUFFIX_COMBINER)$(SUFFIX_MUTEX):
	gcc -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_COMBINER)$(SUFFIX_MUTEX) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_MUTEX) $(CFLAGS)

hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK):
	gcc -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPINLOCK) $(CFLAGS)

hashmin$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST):
	gcc -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST):
	gcc -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPREAD) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

hashmin$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD):
	gcc -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_MUTEX) $(DEFINES_SPREAD) $(CFLAGS)

hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD):
	gcc -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(CFLAGS)

############
# PAGERANK #
############
all_pagerank: pagerank$(SUFFIX_NO_COMBINER) \
			  pagerank$(SUFFIX_COMBINER)$(SUFFIX_MUTEX) \
			  pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK) \
			  pagerank$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST) \
			  pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST) \
			  pagerank$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD) \
			  pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)

pagerank$(SUFFIX_NO_COMBINER):
	gcc -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_NO_COMBINER) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

pagerank$(SUFFIX_COMBINER)$(SUFFIX_MUTEX):
	gcc -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_COMBINER)$(SUFFIX_MUTEX) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_MUTEX) $(CFLAGS)

pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK):
	gcc -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPINLOCK) $(CFLAGS)

pagerank$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST):
	gcc -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST):
	gcc -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPREAD) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

pagerank$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD):
	gcc -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_MUTEX) $(DEFINES_SPREAD) $(CFLAGS)

pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD):
	gcc -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(CFLAGS)

########
# SSSP #
########
all_sssp: sssp$(SUFFIX_NO_COMBINER) \
		  sssp$(SUFFIX_COMBINER)$(SUFFIX_MUTEX) \
		  sssp$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK) \
		  sssp$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST) \
		  sssp$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST) \
		  sssp$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD) \
		  sssp$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)

sssp$(SUFFIX_NO_COMBINER):
	gcc -o $(BIN_DIRECTORY)/sssp$(SUFFIX_NO_COMBINER) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

sssp$(SUFFIX_COMBINER)$(SUFFIX_MUTEX):
	gcc -o $(BIN_DIRECTORY)/sssp$(SUFFIX_COMBINER)$(SUFFIX_MUTEX) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_MUTEX) $(CFLAGS)

sssp$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK):
	gcc -o $(BIN_DIRECTORY)/sssp$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPINLOCK) $(CFLAGS)

sssp$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST):
	gcc -o $(BIN_DIRECTORY)/sssp$(SUFFIX_COMBINER)$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

sssp$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST):
	gcc -o $(BIN_DIRECTORY)/sssp$(SUFFIX_COMBINER)$(SUFFIX_SPREAD)$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPREAD) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

sssp$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD):
	gcc -o $(BIN_DIRECTORY)/sssp$(SUFFIX_COMBINER)$(SUFFIX_MUTEX)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_MUTEX) $(DEFINES_SPREAD) $(CFLAGS)

sssp$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD):
	gcc -o $(BIN_DIRECTORY)/sssp$(SUFFIX_COMBINER)$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_COMBINER) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(CFLAGS)

clean:
	rm -rf $(BIN_DIRECTORY)
