cc=gcc
CC=g++
CFLAGS=-O3 -fopenmp -pthread -Wall -Wextra -Werror -Wfatal-errors

DEFINES=-DIP_FORCE_DIRECT_MAPPING -DIP_ID_OFFSET=$(IP_ID_OFFSET)
DEFINES_WEIGHTED_EDGES=-DIP_WEIGHTED_EDGES
DEFINES_SPINLOCK=-DIP_USE_SPINLOCK
DEFINES_SPREAD=-DIP_USE_SPREAD
DEFINES_SINGLE_BROADCAST=-DIP_USE_SINGLE_BROADCAST
DEFINES_UNUSED_IN_NEIGHBOURS=-DIP_UNUSED_IN_NEIGHBOURS
DEFINES_UNUSED_OUT_NEIGHBOUR_IDS=-DIP_UNUSED_OUT_NEIGHBOUR_IDS
DEFINES_UNUSED_OUT_NEIGHBOURS=-DIP_UNUSED_OUT_NEIGHBOURS

SUFFIX_WEIGHTED_EDGES=_weighted_edges
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
	 all_cc \
	 all_pagerank \
	 all_sssp

#################
# VERIFICATIONS #
#################
verifications:
	@clear
	@echo "=============";
	@echo "VERIFICATIONS";
	@echo "=============";
	@echo "- Is there an ID offset in the graph you are going to process? \c"; \
		if [ -z ${IP_ID_OFFSET} ]; then \
			echo "The variable IP_ID_OFFSET which indicates the ID offset is not set, so it is considered as 0 by default (= no offset).Type \"export IP_ID_OFFSET=0\" and relaunch the makefile. If you have an other offset, just replace the 0 by the value you want."; \
			exit 1; \
		else \
			if [ "${IP_ID_OFFSET}" = "0" ]; then \
				echo "The offset is set to 0 (= no offset), great :)."; \
			else \
				echo "The offset is set to ${IP_ID_OFFSET}, good. Do you want to force the direct mapping (= vertex with ID X is stored at array element X)?"; \
				if [ -z ${IP_FORCE_DIRECT_MAPPING} ]; then \
					echo "The direct mapping is not set, by default it is considered active."; \
					export IP_FORCE_DIRECT_MAPPING=YES; \
				else \
					if [ "${IP_FORCE_DIRECT_MAPPING}" = "YES" ]; then \
						echo "It is set to yes."; \
					elif [ "${IP_FORCE_DIRECT_MAPPING}" = "NO" ]; then \
						echo "It is set to no."; \
					else \
						echo "It is set to \"${IP_FORCE_DIRECT_MAPPING}\", which is unknown, so it is reset to \"YES\". That is, the direct mapping is active."; \
						export IP_FORCE_DIRECT_MAPPING=YES; \
					fi; \
				fi; \
			fi; \
		fi
		@if [ ! -d "${BIN_DIRECTORY}" ]; then \
			echo "- The directory in which outputing binaries is not existing (i.e: \"${BIN_DIRECTORY}\"), so it is created."; mkdir ${BIN_DIRECTORY}; \
		else \
			echo "- Bin directory already existing, good."; \
		fi
	@echo ""

#############
# UTILITIES #
#############
all_utilities: pre_utilities \
			   contiguouer \
			   contiguouerASCII \
			   graph_converter \
			   graph_converter_ligra \
			   all_graph_generators

pre_utilities:
	@echo "=========";
	@echo "UTILITIES";
	@echo "=========";

contiguouer:
	$(CC) -o $(BIN_DIRECTORY)/contiguouer $(SRC_DIRECTORY)/graph_converters/contiguouer.cpp -O2 -std=c++11

contiguouerASCII:
	$(CC) -o $(BIN_DIRECTORY)/contiguouerASCII $(SRC_DIRECTORY)/graph_converters/contiguouerASCII.cpp -O2 -std=c++11

graph_converter:
	$(CC) -o $(BIN_DIRECTORY)/graph_converter $(SRC_DIRECTORY)/graph_converters/graph_converter.cpp -O2 

graph_converter_ligra:
	$(CC) -o $(BIN_DIRECTORY)/graph_converter_ligra $(SRC_DIRECTORY)/graph_converters/graph_converter_ligra.cpp -O2 -DIP_ID_OFFSET=$(IP_ID_OFFSET)

all_graph_generators: graph_generator_iPregel \
					  graph_generator_femtograph \
					  graph_generator_ligra \
					  graph_generator_graphchi

graph_generator_iPregel:
	$(CC) -o $(BIN_DIRECTORY)/graph_generator_iPregel $(SRC_DIRECTORY)/graph_generators/graph_generator_iPregel.cpp -O2 -std=c++11

graph_generator_femtograph:
	$(CC) -o $(BIN_DIRECTORY)/graph_generator_femtograph $(SRC_DIRECTORY)/graph_generators/graph_generator_femtograph.cpp -O2 -std=c++11

graph_generator_ligra:
	$(CC) -o $(BIN_DIRECTORY)/graph_generator_ligra $(SRC_DIRECTORY)/graph_generators/graph_generator_ligra.cpp -O2 -std=c++11

graph_generator_graphchi:
	$(CC) -o $(BIN_DIRECTORY)/graph_generator_graphchi $(SRC_DIRECTORY)/graph_generators/graph_generator_graphchi.cpp -O2 -std=c++11

###########
# HASHMIN #
###########

all_cc: pre_cc \
			 cc \
			 cc$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 cc$(SUFFIX_SPREAD) \
			 cc$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 cc$(SUFFIX_SPINLOCK) \
			 cc$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 cc$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) \
			 cc$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) \
			 cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOURS) \
			 cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD)

pre_cc:
	@echo "================="; \
	echo "Compiling cc"; \
	echo "=================";

cc:
	$(cc) -o $(BIN_DIRECTORY)/cc $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

cc$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

cc$(SUFFIX_SPREAD):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

cc$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

cc$(SUFFIX_SPINLOCK):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(CFLAGS)

cc$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

cc$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(CFLAGS)

cc$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_UNUSED_OUT_NEIGHBOURS) $(CFLAGS)

cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD):
	$(cc) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_SPREAD) $(CFLAGS)

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
	@echo "=================="; \
	echo "Compiling pagerank"; \
	echo "==================";

pagerank:
	$(cc) -o $(BIN_DIRECTORY)/pagerank $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

pagerank$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

pagerank$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS):
	$(cc) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_OUT_NEIGHBOUR_IDS) $(CFLAGS)

pagerank$(SUFFIX_SPINLOCK):
	$(cc) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(CFLAGS)

pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS):
	$(cc) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_OUT_NEIGHBOUR_IDS) $(CFLAGS)

pagerank$(SUFFIX_SINGLE_BROADCAST):
	$(cc) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

pagerank$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS):
	$(cc) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOUR_IDS) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_UNUSED_OUT_NEIGHBOUR_IDS) $(CFLAGS)

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
		  sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOURS) \
		  sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD)

pre_sssp:
	@echo "=============="; \
	echo "Compiling sssp"; \
	echo "==============";

sssp:
	$(cc) -o $(BIN_DIRECTORY)/sssp $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(CFLAGS)

sssp$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SPREAD):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

sssp$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(CFLAGS)

sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPINLOCK)$(SUFFIX_SPREAD)$(SUFFIX_UNUSED_IN_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=gnu99 $(DEFINES) $(DEFINES_SPINLOCK) $(DEFINES_SPREAD) $(DEFINES_UNUSED_IN_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOURS):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_UNUSED_OUT_NEIGHBOURS) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_UNUSED_OUT_NEIGHBOURS) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD):
	$(cc) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) -std=c99 $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

#########
# CLEAN #
#########
clean:
	rm -rf $(BIN_DIRECTORY)
