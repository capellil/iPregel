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

void mp_deserialise(FILE* f)
{
	MP_VERTEX_ID_TYPE src;
	MP_VERTEX_ID_TYPE dest;
	while(fscanf(f, "%u %u", &src, &dest) == 2)
	{
		mp_add_edge(src, dest);
	}
	fclose(f);
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
		printf("Incorrect number of parameters, expecting: %s <inputFile> <outputFile>.\n", argv[0]);
		return -1;
	}

	////////////////////
	// INITILISATION //
	//////////////////
	FILE* f_in = fopen(argv[1], "r");
	if(!f_in)
	{
		perror("File opening failed.");
		return -1;
	}
	MP_VERTEX_ID_TYPE number_of_vertices;
	MP_VERTEX_ID_TYPE number_of_edges;
	if(fscanf(f_in, "%u %u", &number_of_vertices, &number_of_edges) != 2)
	{
		perror("Could not read the number of vertices and number of edges.");
		return -1;
	}
	printf("|V| = %u, |E| = %u.\n", number_of_vertices, number_of_edges);
	mp_init(f_in, number_of_vertices, number_of_edges);

	//////////
	// RUN //
	////////
	//mp_set_id_offset(1);
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
