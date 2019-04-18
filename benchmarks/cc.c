#include <stdlib.h>

typedef unsigned int IP_VERTEX_ID_TYPE;
typedef IP_VERTEX_ID_TYPE IP_NEIGHBOUR_COUNT_TYPE;
typedef IP_VERTEX_ID_TYPE IP_MESSAGE_TYPE;
typedef IP_VERTEX_ID_TYPE IP_VALUE_TYPE;
#include "iPregel.h"

void ip_compute(struct ip_vertex_t* v)
{
	if(ip_is_first_superstep())
	{
		v->value = v->id;
		ip_broadcast(v, v->value);
	}
	else
	{
		IP_MESSAGE_TYPE valueTemp = v->value;
		IP_MESSAGE_TYPE message_value;
		while(ip_get_next_message(v, &message_value))
		{
			if(v->value > message_value)
			{
				v->value = message_value;
			}
		}
		if(valueTemp != v->value)
		{
			ip_broadcast(v, v->value);
		}
	}
	ip_vote_to_halt(v);
}

void ip_combine(IP_MESSAGE_TYPE* a, IP_MESSAGE_TYPE b)
{
	if(*a > b)
	{
		*a = b;
	}
}

void ip_serialise_vertex(FILE* f, struct ip_vertex_t* v)
{
	ip_safe_fwrite(&v->id, sizeof(IP_VERTEX_ID_TYPE), 1, f);
	ip_safe_fwrite(&v->value, sizeof(IP_MESSAGE_TYPE), 1, f);
}

int main(int argc, char* argv[])
{
	if(argc != 4) 
	{
		printf("Incorrect number of parameters, expecting: %s <inputFile> <outputFile> <number_of_threads>.\n", argv[0]);
		return -1;
	}

	////////////////////
	// INITILISATION //
	//////////////////
	bool directed = true;
	bool weighted = false;
	ip_init(argv[1], atoi(argv[3]), directed, weighted);

	//////////
	// RUN //
	////////
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
