#include <stdlib.h>
#include <stdio.h>

typedef unsigned int MP_VERTEX_ID_TYPE;
typedef MP_VERTEX_ID_TYPE MP_MESSAGE_TYPE;
typedef unsigned int MP_NEIGHBOURS_COUNT_TYPE;
#include "my_pregel_preamble.h"
struct mp_vertex_t
{
	MP_VERTEX_STRUCTURE
	MP_MESSAGE_TYPE value;
};
#include "my_pregel_postamble.h"

void mp_combine(MP_MESSAGE_TYPE* a, MP_MESSAGE_TYPE b)
{
	if(*a > b)
	{
		*a = b;
	}
}

void mp_deserialise_vertex(FILE* f)
{
	MP_VERTEX_ID_TYPE vertex_id;
	void* buffer_out_neighbours = NULL;
	unsigned int buffer_out_neighbours_count = 0;
	void* buffer_in_neighbours = NULL;
	unsigned int buffer_in_neighbours_count = 0;

	mp_safe_fread(&vertex_id, sizeof(MP_VERTEX_ID_TYPE), 1, f); 
	mp_safe_fread(&buffer_out_neighbours_count, sizeof(unsigned int), 1, f); 
	if(buffer_out_neighbours_count > 0)
	{
		buffer_out_neighbours = (MP_VERTEX_ID_TYPE*)mp_safe_malloc(sizeof(MP_VERTEX_ID_TYPE) * buffer_out_neighbours_count);
		mp_safe_fread(buffer_out_neighbours, sizeof(MP_VERTEX_ID_TYPE), buffer_out_neighbours_count, f); 
	}
	mp_safe_fread(&buffer_in_neighbours_count, sizeof(unsigned int), 1, f); 
	if(buffer_in_neighbours_count > 0)
	{
		buffer_in_neighbours = (MP_VERTEX_ID_TYPE*)mp_safe_malloc(sizeof(MP_VERTEX_ID_TYPE) * buffer_in_neighbours_count);
		mp_safe_fread(buffer_in_neighbours, sizeof(MP_VERTEX_ID_TYPE), buffer_in_neighbours_count, f); 
	}

	mp_add_vertex(vertex_id, buffer_out_neighbours, buffer_out_neighbours_count, buffer_in_neighbours, buffer_in_neighbours_count);
}

void mp_serialise_vertex(FILE* f, struct mp_vertex_t* v)
{
	(void)(f);
	(void)(v);
}

void mp_compute(struct mp_vertex_t* v)
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

	size_t number_of_vertices = 0;
	if(fread(&number_of_vertices, sizeof(unsigned int), 1, f) != 1)
	{
		perror("Could not read the number of vertices.");
		exit(-1);
	}
	mp_init(f, number_of_vertices);

	struct mp_vertex_t* temp_vertex = NULL;
	MP_NEIGHBOURS_COUNT_TYPE k = 0;
	for(size_t i = 0; i < number_of_vertices; i++)
	{
		temp_vertex = mp_get_vertex_by_location(i);
		for(MP_NEIGHBOURS_COUNT_TYPE j = 0; j < temp_vertex->out_neighbours_count; j++)
		{
			k = 0;
			while(k < mp_get_vertex_by_id(temp_vertex->out_neighbours[j])->in_neighbours_count && mp_get_vertex_by_id(temp_vertex->out_neighbours[j])->in_neighbours[k] != i)
			{
				k++;
			}

			if(k >= mp_get_vertex_by_id(temp_vertex->out_neighbours[j])->in_neighbours_count)
			{
				printf("Failure for vertex %zu.\n", i);
				return -1;
			}
		}
	}

	return EXIT_SUCCESS;
}
