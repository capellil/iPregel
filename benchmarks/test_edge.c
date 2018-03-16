#include <stdlib.h>

typedef unsigned int IP_VERTEX_ID_TYPE;
typedef double IP_MESSAGE_TYPE;
typedef unsigned int IP_NEIGHBOURS_COUNT_TYPE;
typedef unsigned int IP_EDGE_WEIGHT_TYPE;
#include "my_pregel_preamble.h"
struct mp_vertex_t
{
	IP_VERTEX_STRUCTURE
};
#include "my_pregel_postamble.h"

void mp_compute(struct mp_vertex_t* me)
{
	if(me->id == 0)
	{
		for(IP_NEIGHBOURS_COUNT_TYPE i = 0; i < me->out_neighbours_count; i++)
			printf("My ith weight is %u.\n", me->edge_weights[i]);
	}
}

void mp_combine(IP_MESSAGE_TYPE* a, IP_MESSAGE_TYPE b)
{
	(void)a;
	(void)b;
}

void mp_deserialise(FILE* f)
{
	IP_VERTEX_ID_TYPE src;
	IP_VERTEX_ID_TYPE dest;
	while(fscanf(f, "%u %u", &src, &dest) == 2)
	{
		mp_add_edge(src, dest);
	}
	fclose(f);
}

void mp_serialise_vertex(FILE* f, struct mp_vertex_t* v)
{
	(void)f;
	(void)v;
}

int main(int argc, char* argv[])
{
	if(argc != 3) 
	{
		printf("Incorrect number of parameters, expecting: %s <inputFile> <outputFile>.\n", argv[0]);
		return -1;
	}

	////////////////////
	// INITILISATION //
	//////////////////
	IP_VERTEX_ID_TYPE number_of_vertices;
	IP_VERTEX_ID_TYPE number_of_edges;
	FILE* f_in = fopen(argv[1], "r");
	
	if(!f_in)
	{
		perror("File opening failed.");
		return EXIT_FAILURE;
	}
	if(fscanf(f_in, "%u %u", &number_of_vertices, &number_of_edges) != 2)
	{
		perror("Could not read the number of vertices and number of edges.");
		return EXIT_FAILURE;
	}
	printf("|V| = %u, |E| = %u.\n", number_of_vertices, number_of_edges);
	mp_init(f_in, number_of_vertices, number_of_edges);

	//////////
	// RUN //
	////////
	mp_run();

	//////////////
	// DUMPING //
	////////////
	FILE* f_out = fopen(argv[4], "w");
	if(!f_out)
	{
		perror("File opening failed.");
		return -1;
	}
	mp_dump(f_out);

	return EXIT_SUCCESS;
}

