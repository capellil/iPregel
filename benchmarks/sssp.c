#include <stdlib.h>
#include <limits.h>

typedef unsigned int VERTEX_ID;
typedef unsigned int MESSAGE_TYPE;
const VERTEX_ID start_vertex = 1;
#include "my_pregel_preamble.h"
struct vertex_t
{
	VERTEX_STRUCTURE
	MESSAGE_TYPE value;
};
#include "my_pregel_postamble.h"

void compute(struct vertex_t* v)
{
	unsigned int min_dist = (v->id == start_vertex ? 0 : UINT_MAX);
	unsigned int value = UINT_MAX;

	while(get_next_message(v, &value))
	{
		combine(&min_dist, &value);
	}
	
	if(min_dist < v->value)
	{
		v->value = min_dist;
		broadcast(v, min_dist + 1);
	}

	vote_to_halt(v);
}

void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b)
{
	if(*a > *b)
	{
		*a = *b;
	}
}

void deserialise_vertex(FILE* f, struct vertex_t* v)
{
	size_t fread_size = fread(&v->id, sizeof(VERTEX_ID), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from deserialise vertex.\n");
		exit(-1);
	}
	fread_size = fread(&v->out_neighbours_count, sizeof(unsigned int), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from deserialise vertex.\n");
		exit(-1);
	}
	if(v->out_neighbours_count > 0)
	{
		v->out_neighbours = (unsigned int*)safe_malloc(sizeof(VERTEX_ID) * v->out_neighbours_count);
		fread_size = fread(&v->out_neighbours[0], sizeof(VERTEX_ID), v->out_neighbours_count, f);
		if(fread_size != v->out_neighbours_count)
		{
			printf("Error in fread from deserialise vertex.\n");
			exit(-1);
		}
	}
	fread_size = fread(&v->in_neighbours_count, sizeof(unsigned int), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from deserialise vertex.\n");
		exit(-1);
	}
	if(v->in_neighbours_count > 0)
	{
		v->in_neighbours = (unsigned int*)safe_malloc(sizeof(VERTEX_ID) * v->in_neighbours_count);
		fread_size = fread(&v->in_neighbours[0], sizeof(VERTEX_ID), v->in_neighbours_count, f);
		if(fread_size != v->in_neighbours_count)
		{
			printf("Error in fread from deserialise vertex.\n");
			exit(-1);
		}
	}
	v->value = UINT_MAX;
	if(v->id != start_vertex)
	{
		v->active = false;
	}
}

void serialise_vertex(FILE* f, struct vertex_t* v)
{
	(void)(f);
	(void)(v);
}

int main(int argc, char* argv[])
{
	if(argc != 3) 
	{
		printf("Incorrect number of parameters.\n");
		return -1;
	}

	FILE* f_in = fopen(argv[1], "rb");
	if(!f_in)
	{
		perror("File opening failed.");
		return -1;
	}
	
	FILE* f_out = fopen(argv[2], "wb");
	if(!f_out)
	{
		perror("File opening failed.");
		return -1;
	}

	unsigned int number_of_vertices = 0;
	if(fread(&number_of_vertices, sizeof(unsigned int), 1, f_in) != 1)
	{
		perror("Could not read the number of vertices.");
		exit(-1);
	}
	init(f_in, number_of_vertices);
	run();
	dump(f_out);

	return EXIT_SUCCESS;
}
