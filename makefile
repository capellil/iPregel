# C++ Compiler used (must be C++11 compliant)
c++=g++

# C Compiler used (must be C11 compliant)
CC=gcc

# Flags to use for all versions (excluding potential iPregel defines)
CFLAGS=-std=c11 -O3 -fopenmp -Wall -Wextra -Werror -Wfatal-errors
CFLAGS_FOR_UTILITIES=-O2 -std=c++11

DEFINES=-DIP_FORCE_DIRECT_MAPPING -DIP_ID_OFFSET=$(IP_ID_OFFSET) #-DIP_ENABLE_THREAD_PROFILING
DEFINES_WEIGHTED_EDGES=-DIP_WEIGHTED_EDGES
DEFINES_SPINLOCK=-DIP_USE_SPINLOCK
DEFINES_SPREAD=-DIP_USE_SPREAD
DEFINES_SINGLE_BROADCAST=-DIP_USE_SINGLE_BROADCAST

SUFFIX_WEIGHTED_EDGES=_weighted_edges
SUFFIX_SPINLOCK=_spinlock
SUFFIX_SPREAD=_spread
SUFFIX_SINGLE_BROADCAST=_single_broadcast

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
	@echo "";

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
	c++ -o $(BIN_DIRECTORY)/contiguouer $(SRC_DIRECTORY)/graph_converters/contiguouer.cpp $(CFLAGS_FOR_UTILITIES)

contiguouerASCII:
	c++ -o $(BIN_DIRECTORY)/contiguouerASCII $(SRC_DIRECTORY)/graph_converters/contiguouerASCII.cpp $(CFLAGS_FOR_UTILITIES)

graph_converter:
	c++ -o $(BIN_DIRECTORY)/graph_converter $(SRC_DIRECTORY)/graph_converters/graph_converter.cpp $(CFLAGS_FOR_UTILITIES)

graph_converter_ligra:
	c++ -o $(BIN_DIRECTORY)/graph_converter_ligra $(SRC_DIRECTORY)/graph_converters/graph_converter_ligra.cpp $(CFLAGS_FOR_UTILITIES) -DIP_ID_OFFSET=$(IP_ID_OFFSET)

all_graph_generators: graph_generator_femtograph \
					  graph_generator_ligra \
					  graph_generator_graphchi

graph_generator_femtograph:
	c++ -o $(BIN_DIRECTORY)/graph_generator_femtograph $(SRC_DIRECTORY)/graph_generators/graph_generator_femtograph.cpp $(CFLAGS_FOR_UTILITIES)

graph_generator_ligra:
	c++ -o $(BIN_DIRECTORY)/graph_generator_ligra $(SRC_DIRECTORY)/graph_generators/graph_generator_ligra.cpp $(CFLAGS_FOR_UTILITIES)

graph_generator_graphchi:
	c++ -o $(BIN_DIRECTORY)/graph_generator_graphchi $(SRC_DIRECTORY)/graph_generators/graph_generator_graphchi.cpp $(CFLAGS_FOR_UTILITIES)

###########
# HASHMIN #
###########

all_cc: pre_cc \
		cc \
		cc$(SUFFIX_SPREAD) \
		cc$(SUFFIX_SINGLE_BROADCAST) \
		cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) \

pre_cc:
	@echo "================="; \
	echo "Compiling cc"; \
	echo "=================";

cc:
	$(CC) -o $(BIN_DIRECTORY)/cc $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) $(DEFINES) $(CFLAGS)

cc$(SUFFIX_SPREAD):
	$(CC) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

cc$(SUFFIX_SINGLE_BROADCAST):
	$(CC) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD):
	$(CC) -o $(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/cc.c -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_SPREAD) $(CFLAGS)

############
# PAGERANK #
############
all_pagerank: pre_pagerank \
			  pagerank \
			  pagerank$(SUFFIX_SINGLE_BROADCAST) \

pre_pagerank:
	@echo "=================="; \
	echo "Compiling pagerank"; \
	echo "==================";

pagerank:
	$(CC) -o $(BIN_DIRECTORY)/pagerank $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) $(DEFINES) $(CFLAGS)

pagerank$(SUFFIX_SINGLE_BROADCAST):
	$(CC) -o $(BIN_DIRECTORY)/pagerank$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/pagerank.c -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

########
# SSSP #
########
all_sssp: pre_sssp \
		  sssp \
		  sssp$(SUFFIX_SPREAD) \
		  sssp$(SUFFIX_SINGLE_BROADCAST) \
		  sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) \

pre_sssp:
	@echo "=============="; \
	echo "Compiling sssp"; \
	echo "==============";

sssp:
	$(CC) -o $(BIN_DIRECTORY)/sssp $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) $(DEFINES) $(CFLAGS)

sssp$(SUFFIX_SPREAD):
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST):
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD):
	$(CC) -o $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) $(BENCHMARKS_DIRECTORY)/sssp.c -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

#########
# CLEAN #
#########
clean:
	rm -rf $(BIN_DIRECTORY)
