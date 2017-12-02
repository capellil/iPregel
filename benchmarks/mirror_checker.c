#include <stdlib.h>
#include <stdio.h>

typedef unsigned int VERTEX_ID;
typedef unsigned int MESSAGE_TYPE;
#include "my_pregel_preamble.h"
struct vertex_t
{
	VERTEX_STRUCTURE
	MESSAGE_TYPE value;
};
#include "my_pregel_postamble.h"

void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b)
{
	if(*a > *b)
	{
		*a = *b;
	}
}

void deserialise_vertex(FILE* f)
{
	VERTEX_ID vertex_id;
	void* buffer_out_neighbours = NULL;
	unsigned int buffer_out_neighbours_count = 0;
	void* buffer_in_neighbours = NULL;
	unsigned int buffer_in_neighbours_count = 0;

	if(fread(&vertex_id, sizeof(VERTEX_ID), 1, f) != 1)
	{
		printf("Error in fread from deserialise vertex: ID.\n");
		exit(-1);
	}

	if(fread(&buffer_out_neighbours_count, sizeof(unsigned int), 1, f) != 1)
	{
		printf("Error in fread from deserialise vertex: buffer_out_neighbours_size.\n");
		exit(-1);
	}
	if(buffer_out_neighbours_count > 0)
	{
		buffer_out_neighbours = (VERTEX_ID*)safe_malloc(sizeof(VERTEX_ID) * buffer_out_neighbours_count);
		if(fread(buffer_out_neighbours, sizeof(VERTEX_ID), buffer_out_neighbours_count, f) != buffer_out_neighbours_count)
		{
			printf("Error in fread from deserialise vertex: buffer_out_neighbours.\n");
			exit(-1);
		}
	}

	if(fread(&buffer_in_neighbours_count, sizeof(unsigned int), 1, f) != 1)
	{
		printf("Error in fread from deserialise vertex: buffer_in_neighbours_size.\n");
		exit(-1);
	}
	if(buffer_in_neighbours_count > 0)
	{
		buffer_in_neighbours = (VERTEX_ID*)safe_malloc(sizeof(VERTEX_ID) * buffer_in_neighbours_count);
		if(fread(buffer_in_neighbours, sizeof(VERTEX_ID), buffer_in_neighbours_count, f) != buffer_in_neighbours_count)
		{
			printf("Error in fread from deserialise vertex: buffer_in_neighbours.\n");
			exit(-1);
		}
	}
	add_vertex(vertex_id, buffer_out_neighbours, buffer_out_neighbours_count, buffer_in_neighbours, buffer_in_neighbours_count);
}

void serialise_vertex(FILE* f, struct vertex_t* v)
{
	(void)(f);
	(void)(v);
}

void compute(struct vertex_t* v)
{
	(void)(v);
}

int main(int argc, char* argv[])
{
	if(argc != 2) 
	{
		printf("Incorrect number of parameters.\n");
		return -1;
	}

	FILE* f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		printf("Cannot open the input file.\n");
		return -1;
	}

	unsigned int number_of_vertices = 0;
	if(fread(&number_of_vertices, sizeof(unsigned int), 1, f) != 1)
	{
		perror("Could not read the number of vertices.");
		exit(-1);
	}
	init(f, number_of_vertices);

	unsigned int k = 0;
	for(unsigned int i = 1; i <= number_of_vertices; i++)
	{
		//printf("Checking vertex %u:", i);
		for(unsigned int j = 0; j < all_vertices[i].out_neighbours_count; j++)
		{
			k = 0;
			while(k < all_vertices[all_vertices[i].out_neighbours[j]].in_neighbours_count && all_vertices[all_vertices[i].out_neighbours[j]].in_neighbours[k] != i)
			{
				k++;
			}

			if(k >= all_vertices[all_vertices[i].out_neighbours[j]].in_neighbours_count)
			{
				printf("Failure for vertex %u.\n", i);
				return -1;
			}
		}
		//printf("succeeded.\n");
	}

	return EXIT_SUCCESS;
}
