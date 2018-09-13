#include <stdlib.h>

typedef unsigned int IP_VERTEX_ID_TYPE;
typedef double IP_MESSAGE_TYPE;
typedef unsigned int IP_NEIGHBOURS_COUNT_TYPE;
const unsigned int ROUND = 30;
#include "iPregel_preamble.h"
struct ip_vertex_t
{
	IP_VERTEX_STRUCTURE
	IP_MESSAGE_TYPE value;
};
#include "iPregel_postamble.h"
double ratio;
double initial_value;

void ip_compute(struct ip_vertex_t* v)
{
	if(ip_is_first_superstep())
	{
		v->value = initial_value;
	}
	else
	{
		IP_MESSAGE_TYPE sum = 0.0;
		IP_MESSAGE_TYPE value_teip;
		while(ip_get_next_message(v, &value_teip))
		{
			sum += value_teip;
		}

		value_teip = ratio + 0.85 * sum;
		v->value = value_teip;
	}

	if(ip_get_superstep() < ROUND)
	{
		ip_broadcast(v, v->value / v->out_neighbours_count);
	}
	else
	{
		ip_vote_to_halt(v);
	}
}

void ip_combine(IP_MESSAGE_TYPE* a, IP_MESSAGE_TYPE b)
{
	*a += b;
}

void ip_deserialise(FILE* f)
{
	IP_VERTEX_ID_TYPE src;
	IP_NEIGHBOURS_COUNT_TYPE count;
	IP_VERTEX_ID_TYPE* neighbours;

	while(fread(&src, sizeof(IP_VERTEX_ID_TYPE), 1, f) == 1)
	{
		ip_safe_fread(&count, sizeof(IP_NEIGHBOURS_COUNT_TYPE), 1, f);
		if(count > 0)
		{
			neighbours = ip_safe_malloc(count * sizeof(IP_VERTEX_ID_TYPE));
			ip_safe_fread(neighbours, sizeof(IP_VERTEX_ID_TYPE), count, f);
			for(IP_NEIGHBOURS_COUNT_TYPE i = 0; i < count; i++)
			{
				ip_add_edge(src, neighbours[i]);
			}
			ip_safe_free(neighbours);
		}
	}
	fclose(f);
}

void ip_serialise_vertex(FILE* f, struct ip_vertex_t* v)
{
	ip_safe_fwrite(&v->id, sizeof(IP_VERTEX_ID_TYPE), 1, f);
	ip_safe_fwrite(&v->value, sizeof(IP_MESSAGE_TYPE), 1, f);
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
	FILE* f_in = fopen(argv[1], "rb");
	if(!f_in)
	{
		perror("File opening failed.");
		return -1;
	}
	IP_VERTEX_ID_TYPE number_of_vertices;
	IP_VERTEX_ID_TYPE number_of_edges;
	ip_safe_fread(&number_of_vertices, sizeof(IP_VERTEX_ID_TYPE), 1, f_in);
	ip_safe_fread(&number_of_edges, sizeof(IP_VERTEX_ID_TYPE), 1, f_in);
	printf("|V| = %u, |E| = %u.\n", number_of_vertices, number_of_edges);
	ip_init(f_in, number_of_vertices, number_of_edges);

	//////////
	// RUN //
	////////
	ratio = 0.15 / ip_get_vertices_count();
	initial_value = 1.0 / ip_get_vertices_count();
	ip_run();

	//////////////
	// DUMPING //
	////////////
	FILE* f_out = fopen(argv[2], "w");
	if(!f_out)
	{
		perror("File opening failed.");
		return -1;
	}
	ip_dump(f_out);

	return EXIT_SUCCESS;
}

