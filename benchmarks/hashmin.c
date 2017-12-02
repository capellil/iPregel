#include <stdlib.h>

typedef unsigned int VERTEX_ID;
typedef unsigned int MESSAGE_TYPE;
#include "my_pregel_preamble.h"
struct vertex_t
{
	VERTEX_STRUCTURE
	MESSAGE_TYPE value;
};
#include "my_pregel_postamble.h"

void compute(struct vertex_t* v)
{
	if(superstep == 0)
	{
		v->value = v->id;
		if(v->in_neighbours_count > 0)
		{
			for(unsigned int i = 0; i < v->in_neighbours_count; i++)
			{
				if(v->in_neighbours[i] < v->value)
				{   
					v->value = v->in_neighbours[i];
				}
			}
		}
		if(v->out_neighbours_count > 0)
		{
			for(unsigned int i = 0; i < v->out_neighbours_count; i++)
			{
				if(v->out_neighbours[i] < v->value)
				{   
					v->value = v->out_neighbours[i];
				}
			}
		}

		broadcast(v, v->value);
		vote_to_halt(v);
	}
	else
	{
		MESSAGE_TYPE valueTemp = v->value;
		MESSAGE_TYPE message_value;
		while(get_next_message(v, &message_value))
		{
			if(v->value > message_value)
			{
				v->value = message_value;
			}
		}

		if(valueTemp != v->value)
		{
			broadcast(v, v->value);
		}

		vote_to_halt(v);
	}
}

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
	fwrite(&v->id, sizeof(VERTEX_ID), 1, f);
	fwrite(&v->value, sizeof(MESSAGE_TYPE), 1, f);
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
