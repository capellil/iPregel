#include <stdlib.h>
#include <limits.h>
#include <float.h>

typedef unsigned int MP_VERTEX_ID_TYPE;
typedef MP_VERTEX_ID_TYPE MP_MESSAGE_TYPE;
typedef unsigned int MP_NEIGHBOURS_COUNT_TYPE;
const MP_VERTEX_ID_TYPE start_vertex = 2;
#include "my_pregel_preamble.h"
struct mp_vertex_t
{
	MP_VERTEX_STRUCTURE
	MP_MESSAGE_TYPE dist;
};
#include "my_pregel_postamble.h"

void mp_compute(struct mp_vertex_t* v)
{
	if(mp_is_first_superstep())
	{
		if(v->id == start_vertex)
		{
			v->dist = 0;
			mp_broadcast(v, v->dist + 1);
		}
		else
		{
			v->dist = UINT_MAX;
		}		
	}
	else
	{
		MP_MESSAGE_TYPE m_initial = UINT_MAX;
		MP_MESSAGE_TYPE m;
		while(mp_get_next_message(v, &m))
		{
			if(m_initial > m)
			{
				m_initial = m;
			}
		}
		if(m_initial < v->dist)
		{
			v->dist = m_initial;
			mp_broadcast(v, m_initial + 1);
		}
	}

	mp_vote_to_halt(v);
}

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
