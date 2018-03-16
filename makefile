CC=gcc
CFLAGS=-O2 -fopenmp -pthread -Wall -Wextra -Werror -Wfatal-errors

DEFINES=-DFORCE_DIRECT_MAPPING -DOMP_NUM_THREADS=$(OMP_NUM_THREADS) -DIP_ID_OFFSET=$(IP_ID_OFFSET)
DEFINES_SPINLOCK=-DIP_USE_SPINLOCK
DEFINES_SPREAD=-DIP_USE_SPREAD
DEFINES_SINGLE_BROADCAST=-DIP_USE_SINGLE_BROADCAST
DEFINES_UNUSED_IN_NEIGHBOURS=-DIP_UNUSED_IN_NEIGHBOURS
DEFINES_UNUSED_OUT_NEIGHBOUR_IDS=-DIP_UNUSED_OUT_NEIGHBOUR_IDS
DEFINES_UNUSED_OUT_NEIGHBOURS=-DIP_UNUSED_OUT_NEIGHBOURS

SUFFIX_SPINLOCK=_spinlock
SUFFIX_SPREAD=_spread
SUFFIX_SINGLE_BROADCAST=_single_broadcast
SUFFIX_UNUSED_IN_NEIGHBOURS=_unused_in_neighbours
SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS=_unused_out_neighbour_ids
SUFFIX_UNUSED_OUT_NEIGHBOURS=_unused_out_neighbours

SRC_DIRECTORY=src
BENCHMARKS_DIRECTORY=benchmarks
BIN_DIRECTORY=bin
COMPILATION_PREFIX="    --> \c"

default: all

all: verifications \
	 all_utilities \
	 all_hashmin \
	 all_pagerank \
	 all_sssp

#################
# VERIFICATIONS #
#################
verifications:
	@clear
	@echo "=============";
	@echo "VERIFICATIONS";
	@echo "=============\n";
	@echo "- Is there direct mapping (i.e: is \"FORCE_DIRECT_MAPPING\" defined)? \c"; if [ -z ${FORCE_DIRECT_MAPPING} ]; then \
		echo "It is not defined, please tell if there is a direct mapping (\"YES\") or not (\"NO\") with \"export FORCE_DIRECT_MAPPING=<yourChoice>. Aborting..."; exit 1; \
	elif [ "${FORCE_DIRECT_MAPPING}" = "YES" ]; then \
		echo "Yes, so if an id offset is defined it is ignored."; \
	elif [ "${FORCE_DIRECT_MAPPING}" = "NO" ]; then \
		echo "Yes it is defined, and is set to no. So you must give me the id offset, that is, the minimum id of your vertices. \c"; \
		if [ -z "${IP_ID_OFFSET}" ]; then \
			echo "IP_ID_OFFSET is not set, please set it to the offset of ID present in the graph with 'export IP_ID_OFFSET=<idOffset>'."; exit 1; \
		else \
			echo "Good, the IP_ID_OFFSET is already set to '${IP_ID_OFFSET}'."; \
		fi \
	else \
		echo "I do not know, it is set to \"${FORCE_DIRECT_MAPPING}\" but it should be either \"YES\" or \"NO\" with \"export FORCE_DIRECT_MAPPING=<yourChoice>. Aborting..."; exit 1; \
	fi
	@if [ -z "${OMP_NUM_THREADS}" ]; then echo "- OMP_NUM_THREADS is not set, please set it to the number of threads usable by OpenMP with 'export OMP_NUM_THREADS=<#threads>'."; exit 1; else echo "- OMP_NUM_THREADS set to '${OMP_NUM_THREADS}'"; fi
	@if [ ! -d "${BIN_DIRECTORY}" ]; then echo "- The directory in which outputing binaries is not existing (i.e: \"${BIN_DIRECTORY}\"), so it is created."; mkdir ${BIN_DIRECTORY}; else echo "- Bin directory already existing, good."; fi
	@echo ""

#############
# UTILITIES #
#############
all_utilities: pre_utilities \
			   contiguouer \
			   contiguouerASCII \
			   graph_converter \
			   graph_generator

pre_utilities:
	@echo "=========";
	@echo "UTILITIES";
	@echo "=========\n";

contiguouer:
	@echo $(COMPILATION_PREFIX);
	g++ -o $(BIN_DIRECTORY)/contiguouer $(SRC_DIRECTORY)/contiguouer.cpp -O2 -std=c++11

contiguouerASCII:
	@echo $(COMPILATION_PREFIX);
	g++ -o $(BIN_DIRECTORY)/contiguouerASCII $(SRC_DIRECTORY)/contiguouerASCII.cpp -O2 -std=c++11

graph_converter:
	@echo $(COMPILATION_PREFIX);
	g++ -o $(BIN_DIRECTORY)/graph_converter $(SRC_DIRECTORY)/graph_converter.cpp -O2

graph_generator:
	@echo $(COMPILATION_PREFIX);
	g++ -o $(BIN_DIRECTORY)/graph_generator $(SRC_DIRECTORY)/graph_generator.cpp -O2 -std=c++11

###########
# HASHMIN #
###########
all_hashmin: pre_hashmin \
			 hashmin \
			 hashmin$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 hashmin$(SUFFIX_SPREAD) \
			 hashmin$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 hashmin$(SUFFIX_SPINLOCK) \
			 hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) \
			 hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 hashmin$(SUFFIX_SINGLE_BROADCAST) \
			 hashmin$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD)

pre_hashmin:
	@echo "\n================="
	@echo "Compiling hashmin"
	@echo "=================\n"

hashmin:
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

hashmin$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

hashmin$(SUFFIX_SPREAD):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

hashmin$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

hashmin$(SUFFIX_SPINLOCK):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(CFLAGS)

hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(CFLAGS)

hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

hashmin$(SUFFIX_SINGLE_BROADCAST):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

hashmin$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/hashmin$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/hashmin.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_SPREAD) $(CFLAGS)

############
# PAGERANK #
############
all_pagerank: pre_pagerank \
			  pagerank \
			  pagerank$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			  pagerank$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) \
			  pagerank$(SUFFIX_SPINLOCK) \
			  pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			  pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) \
			  pagerank$(SUFFIX_SINGLE_BROADCAST) \
			  pagerank$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS)

pre_pagerank:
	@echo "\n=================="
	@echo "Compiling pagerank"
	@echo "==================\n"

pagerank:
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

pagerank$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

pagerank$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_OUT_NEIGHBOUR_IDS) $(CFLAGS)

pagerank$(SUFFIX_SPINLOCK):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(CFLAGS)

pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_OUT_NEIGHBOUR_IDS) $(CFLAGS)

pagerank$(SUFFIX_SINGLE_BROADCAST):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

pagerank$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_UNUSED_OUT_NEIGHBOUR_IDS) $(CFLAGS)

########
# SSSP #
########
all_sssp: pre_sssp \
		  sssp \
		  sssp$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
		  sssp$(SUFFIX_SPREAD) \
		  sssp$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
		  sssp$(SUFFIX_SPINLOCK) \
		  sssp$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
		  sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) \
		  sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
		  sssp$(SUFFIX_SINGLE_BROADCAST) \
		  sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) \
		  sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD)

pre_sssp:
	@echo "\n=============="
	@echo "Compiling sssp"
	@echo "==============\n"

sssp:
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

sssp$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SPREAD):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

sssp$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_UNUSED_OUT_NEIGHBOUR_IDS) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD):
	@echo $(COMPILATION_PREFIX);
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

#########
# CLEAN #
#########
clean:
	rm -rf $(BIN_DIRECTORY)
