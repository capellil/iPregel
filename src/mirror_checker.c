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
