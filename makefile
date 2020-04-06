# C++ Compiler used (must be C++11 compliant)
c++=g++

# C Compiler used (must be C11 compliant)
CC=gcc

# Flags to use for all versions (excluding potential iPregel defines)
CFLAGS=-std=c11 -O3 -fopenmp -lvmem -Wall -Wextra -Wfatal-errors
CFLAGS_FOR_UTILITIES=-O2 -std=c++11

DEFINES=-DIP_FORCE_DIRECT_MAPPING #-DIP_ENABLE_THREAD_PROFILING
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

COMMON_FILES=$(SRC_DIRECTORY)/iPregel_preamble.h $(SRC_DIRECTORY)/iPregel_postamble.h
COMMON_FILES_COMBINER=$(COMMON_FILES) $(SRC_DIRECTORY)/combiner_preamble.h $(SRC_DIRECTORY)/combiner_postamble.h
COMMON_FILES_COMBINER_SPREAD=$(COMMON_FILES) $(SRC_DIRECTORY)/combiner_spread_preamble.h $(SRC_DIRECTORY)/combiner_spread_postamble.h
COMMON_FILES_COMBINER_SINGLE_BROADCAST=$(COMMON_FILES) $(SRC_DIRECTORY)/combiner_single_broadcast_preamble.h $(SRC_DIRECTORY)/combiner_single_broadcast_postamble.h
COMMON_FILES_COMBINER_SPREAD_AND_SINGLE_BROADCAST=$(COMMON_FILES) $(SRC_DIRECTORY)/combiner_spread_single_broadcast_preamble.h $(SRC_DIRECTORY)/combiner_spread_single_broadcast_postamble.h

default: all

all: $(BIN_DIRECTORY) \
	 all_utilities \
	 all_cc \
	 all_pagerank \
	 all_sssp

#################
# VERIFICATIONS #
#################
$(BIN_DIRECTORY): 
	mkdir -p $@;

#############
# UTILITIES #
#############
all_utilities: $(BIN_DIRECTORY)/contiguouer \
			   $(BIN_DIRECTORY)/contiguouerASCII \
			   $(BIN_DIRECTORY)/graph_converter \
			   $(BIN_DIRECTORY)/graph_converter_ligra \
			   all_graph_generators

$(BIN_DIRECTORY)/contiguouer: $(SRC_DIRECTORY)/graph_converters/contiguouer.cpp
	c++ -o $@ $^ $(CFLAGS_FOR_UTILITIES)

$(BIN_DIRECTORY)/contiguouerASCII: $(SRC_DIRECTORY)/graph_converters/contiguouerASCII.cpp
	c++ -o $@ $^ $(CFLAGS_FOR_UTILITIES)

$(BIN_DIRECTORY)/graph_converter: $(SRC_DIRECTORY)/graph_converters/graph_converter.cpp
	c++ -o $@ $^ $(CFLAGS_FOR_UTILITIES)

$(BIN_DIRECTORY)/graph_converter_ligra: $(SRC_DIRECTORY)/graph_converters/graph_converter_ligra.cpp
	c++ -o $@ $^ $(CFLAGS_FOR_UTILITIES)

all_graph_generators: $(BIN_DIRECTORY)/graph_generator_femtograph \
					  $(BIN_DIRECTORY)/graph_generator_ligra \
					  $(BIN_DIRECTORY)/graph_generator_graphchi

$(BIN_DIRECTORY)/graph_generator_femtograph: $(SRC_DIRECTORY)/graph_generators/graph_generator_femtograph.cpp
	c++ -o $@ $^ $(CFLAGS_FOR_UTILITIES)

$(BIN_DIRECTORY)/graph_generator_ligra: $(SRC_DIRECTORY)/graph_generators/graph_generator_ligra.cpp
	c++ -o $@ $^ $(CFLAGS_FOR_UTILITIES)

$(BIN_DIRECTORY)/graph_generator_graphchi: $(SRC_DIRECTORY)/graph_generators/graph_generator_graphchi.cpp
	c++ -o $@ $^ $(CFLAGS_FOR_UTILITIES)

###########
# HASHMIN #
###########

all_cc: $(BIN_DIRECTORY)/cc \
		$(BIN_DIRECTORY)/cc$(SUFFIX_SPREAD) \
		$(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST) \
		$(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) \

$(BIN_DIRECTORY)/cc: $(BENCHMARKS_DIRECTORY)/cc.c $(COMMON_FILES_COMBINER)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(CFLAGS)

$(BIN_DIRECTORY)/cc$(SUFFIX_SPREAD): $(BENCHMARKS_DIRECTORY)/cc.c $(COMMON_FILES_COMBINER_SPREAD)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

$(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST): $(BENCHMARKS_DIRECTORY)/cc.c $(COMMON_FILES_COMBINER_SINGLE_BROADCAST)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

$(BIN_DIRECTORY)/cc$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD): $(BENCHMARKS_DIRECTORY)/cc.c $(COMMON_FILES_COMBINER_SPREAD_AND_SINGLE_BROADCAST)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(DEFINES_SPREAD) $(CFLAGS)

############
# PAGERANK #
############
all_pagerank: $(BIN_DIRECTORY)/pagerank \
			  $(BIN_DIRECTORY)/pagerank$(SUFFIX_SINGLE_BROADCAST) \

$(BIN_DIRECTORY)/pagerank: $(BENCHMARKS_DIRECTORY)/pagerank.c $(COMMON_FILES_COMBINER)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(CFLAGS)

$(BIN_DIRECTORY)/pagerank$(SUFFIX_SINGLE_BROADCAST): $(BENCHMARKS_DIRECTORY)/pagerank.c $(COMMON_FILES_COMBINER_SINGLE_BROADCAST)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

########
# SSSP #
########
all_sssp: $(BIN_DIRECTORY)/sssp \
		  $(BIN_DIRECTORY)/sssp$(SUFFIX_SPREAD) \
		  $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST) \
		  $(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD) \

$(BIN_DIRECTORY)/sssp: $(BENCHMARKS_DIRECTORY)/sssp.c $(COMMON_FILES_COMBINER)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(CFLAGS)

$(BIN_DIRECTORY)/sssp$(SUFFIX_SPREAD): $(BENCHMARKS_DIRECTORY)/sssp.c $(COMMON_FILES_COMBINER_SPREAD)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SPREAD) $(CFLAGS)

$(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST): $(BENCHMARKS_DIRECTORY)/sssp.c $(COMMON_FILES_COMBINER_SINGLE_BROADCAST)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

$(BIN_DIRECTORY)/sssp$(SUFFIX_SINGLE_BROADCAST)$(SUFFIX_SPREAD): $(BENCHMARKS_DIRECTORY)/sssp.c $(COMMON_FILES_COMBINER_SPREAD_AND_SINGLE_BROADCAST)
	$(CC) -o $@ $< -I$(SRC_DIRECTORY) $(DEFINES) $(DEFINES_SPREAD) $(DEFINES_SINGLE_BROADCAST) $(CFLAGS)

#########
# CLEAN #
#########
clean:
	rm -rf $(BIN_DIRECTORY)
