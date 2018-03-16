#include <stdlib.h>

typedef unsigned int IP_VERTEX_ID_TYPE;
typedef double IP_MESSAGE_TYPE;
typedef unsigned int IP_NEIGHBOURS_COUNT_TYPE;
const unsigned int ROUND = 30;
#include "my_pregel_preamble.h"
struct ip_vertex_t
{
	IP_VERTEX_STRUCTURE
	IP_MESSAGE_TYPE value;
};
#include "my_pregel_postamble.h"
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
	IP_VERTEX_ID_TYPE dest;
	while(fscanf(f, "%u %u", &src, &dest) == 2)
	{
		ip_add_edge(src, dest);
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
	FILE* f_in = fopen(argv[1], "r");
	if(!f_in)
	{
		perror("File opening failed.");
		return -1;
	}
	IP_VERTEX_ID_TYPE number_of_vertices;
	IP_VERTEX_ID_TYPE number_of_edges;
	if(fscanf(f_in, "%u %u", &number_of_vertices, &number_of_edges) != 2)
	{
		perror("Could not read the number of vertices and number of edges.");
		return -1;
	}
	printf("|V| = %u, |E| = %u.\n", number_of_vertices, number_of_edges);
	ip_init(f_in, number_of_vertices, number_of_edges);

	//////////
	// RUN //
	////////
	//ip_set_id_offset(1);
	ratio = 0.15 / ip_get_vertices_count();
	initial_value = 1.0 / ip_get_vertices_count();
	ip_run();

	//////////////
	// DUMPING //
	////////////
	FILE* f_out = fopen(argv[4], "w");
	if(!f_out)
	{
		perror("File opening failed.");
		return -1;
	}
	ip_duip(f_out);

	return EXIT_SUCCESS;
}

