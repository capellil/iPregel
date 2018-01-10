#include <stdlib.h>
#include <limits.h>
#include <float.h>

typedef unsigned int MP_VERTEX_ID_TYPE;
struct sssp_message_t
{
	double dist;
	MP_VERTEX_ID_TYPE from;
};
typedef struct sssp_message_t MP_MESSAGE_TYPE;
typedef unsigned int MP_NEIGHBOURS_COUNT_TYPE;
const MP_VERTEX_ID_TYPE start_vertex = 2;
#include "my_pregel_preamble.h"
struct mp_vertex_t
{
	MP_VERTEX_STRUCTURE
	double dist;
	MP_VERTEX_ID_TYPE from;
};
#include "my_pregel_postamble.h"

void mp_compute(struct mp_vertex_t* v)
{
	if(mp_is_first_superstep())
	{
		if(v->id == start_vertex)
		{
			v->dist = 0;
			v->from = UINT_MAX;
			struct sssp_message_t m;
			m.dist = v->dist;
			m.from = v->id;
			mp_broadcast(v, m);
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
		while(mp_get_next_message(v, &m))
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
			mp_broadcast(v, m_initial);
		}
	}

	mp_vote_to_halt(v);
}

void mp_combine(MP_MESSAGE_TYPE* a, MP_MESSAGE_TYPE* b)
{
	if(a->dist > b->dist)
	{
		a->dist = b->dist;
		a->from = b->from;
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
	
	FILE* f_out = fopen(argv[2], "w");
	if(!f_out)
	{
		perror("File opening failed.");
		return -1;
	}

	size_t number_of_vertices = 0;
	if(fread(&number_of_vertices, sizeof(unsigned int), 1, f_in) != 1)
	{
		perror("Could not read the number of vertices.");
		exit(-1);
	}
	mp_set_id_offset(1);
	mp_init(f_in, number_of_vertices);
	mp_run();
	mp_dump(f_out);

	return EXIT_SUCCESS;
}
