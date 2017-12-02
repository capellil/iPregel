#include <stdlib.h>
#include <limits.h>
#include <float.h>

typedef unsigned int VERTEX_ID;
struct sssp_message_t
{
	double dist;
	VERTEX_ID from;
};
typedef struct sssp_message_t MESSAGE_TYPE;
const VERTEX_ID start_vertex = 2;
#include "my_pregel_preamble.h"
struct vertex_t
{
	VERTEX_STRUCTURE
	double dist;
	VERTEX_ID from;
};
#include "my_pregel_postamble.h"

void compute(struct vertex_t* v)
{
	if(superstep == 0)
	{
		if(v->id == start_vertex)
		{
			v->dist = 0;
			v->from = UINT_MAX;
			struct sssp_message_t m;
			m.dist = v->dist;
			m.from = v->id;
			broadcast(v, m);
		}
		else
		{
			v->dist = DBL_MAX;
			v->from = UINT_MAX;
		}		
	}
	else
	{
		struct sssp_message_t m_initial;
		m_initial.dist = DBL_MAX;
		m_initial.from = UINT_MAX;
		struct sssp_message_t m;
		while(get_next_message(v, &m))
		{
			if(m_initial.dist > m.dist)
			{
				m_initial.dist = m.dist;
				m_initial.from = m.from;
			}
		}
		if(m_initial.dist < v->dist)
		{
			v->dist = m_initial.dist;
			v->from = m_initial.from;
			broadcast(v, m_initial);
		}
	}

	vote_to_halt(v);
}

void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b)
{
	if(a->dist > b->dist)
	{
		a->dist = b->dist;
		a->from = b->from;
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
