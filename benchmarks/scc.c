#include <stdlib.h>
#include <limits.h>

typedef unsigned int VERTEX_ID;
typedef VERTEX_ID MESSAGE_TYPE;
#include "my_pregel_preamble.h"
struct vertex_t
{
	VERTEX_STRUCTURE
	VERTEX_ID min_f;
	VERTEX_ID min_b;
};
#include "my_pregel_postamble.h"

void compute(struct vertex_t* v)
{
	if(meta_superstep == 0)
	{
		if(superstep == 0)
		{
			v->min_f = v->id;
			broadcast(v, v->min_f);
			vote_to_halt(v);
		}
		else
		{
			MESSAGE_TYPE initial_min_f = v->min_f;
			MESSAGE_TYPE message_value;
			while(get_next_message(v, &message_value))
			{
				if(v->min_f > message_value)
				{
					v->min_f = message_value;
				}
			}
	
			if(initial_min_f != v->min_f)
			{
				broadcast(v, v->min_f);
			}
	
			vote_to_halt(v);
		}
	}
	else
	{
		if(superstep == 0)
		{
			if(v->id == v->min_f)
			{
				v->min_b = v->id;
				broadcast(v, v->min_b);
			}
			else
			{
				v->min_b = UINT_MAX;
			}
			vote_to_halt(v);
		}
		else
		{
			MESSAGE_TYPE initial_min_b = v->min_b;
			MESSAGE_TYPE message_value;
			while(get_next_message(v, &message_value))
			{
				if(v->min_b > message_value)
				{
					v->min_b = message_value;
				}
			}
	
			if(initial_min_b != v->min_b)
			{
				broadcast(v, v->min_b);
			}
	
			vote_to_halt(v);
		}
	}
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
}

void serialise_vertex(FILE* f, struct vertex_t* v)
{
	fwrite(&v->id, sizeof(VERTEX_ID), 1, f);
	fwrite(&v->min_f, sizeof(VERTEX_ID), 1, f);
	fwrite(&v->min_b, sizeof(VERTEX_ID), 1, f);
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
	meta_superstep_count = 2;
	run();
	dump(f_out);

	return EXIT_SUCCESS;
}
