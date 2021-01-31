/**
 * @file sssp.c
 * @copyright Copyright (C) 2019 Ludovic Capelli
 * @par License
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * @author Ludovic Capelli
 **/
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <inttypes.h>

typedef uint32_t IP_VERTEX_ID_TYPE;
typedef uint64_t IP_NEIGHBOUR_COUNT_TYPE;
typedef IP_VERTEX_ID_TYPE IP_MESSAGE_TYPE;
typedef IP_VERTEX_ID_TYPE IP_VALUE_TYPE;
#include "iPregel.h"
// For reference DBLP, start_vertex=0
// For reference liveJournal, start_vertex=0
// For reference orkut, start_vertex=2
// For reference Friendster, start_vertex=101
IP_VERTEX_ID_TYPE start_vertex;

void ip_compute(struct ip_vertex_t* v)
{
	if(ip_is_first_superstep())
	{
		if(v->id == start_vertex)
		{
			v->value = 0;
			ip_broadcast(v, v->value + 1);
		}
		else
		{
			v->value = UINT_MAX;
		}		
	}
	else
	{
		IP_MESSAGE_TYPE m_initial = UINT_MAX;
		IP_MESSAGE_TYPE m;
		while(ip_get_next_message(v, &m))
		{
			if(m_initial > m)
			{
				m_initial = m;
			}
		}
		if(m_initial < v->value)
		{
			v->value = m_initial;
			ip_broadcast(v, m_initial + 1);
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
	fprintf(f, "%u: %u\n", v->id, v->value);
}

int main(int argc, char* argv[])
{
	if(argc != 5) 
	{
		printf("Incorrect number of parameters, expecting: %s <inputFile> <outputFile> <number_of_threads> <SSSSP_source_vertex>.\n", argv[0]);
		return -1;
	}

	printf("ApplicationConfiguration:startVertex=%u\n", atoi(argv[4]));

	////////////////////
	// INITILISATION //
	//////////////////
	bool directed = false;
	bool weighted = false;
	start_vertex = atoi(argv[4]);
	ip_init(argv[1], atoi(argv[3]), directed, weighted);

	//////////
	// RUN //
	////////
	ip_run();

	//////////////
	// DUMPING //
	////////////
	FILE* f_out = fopen(argv[2], "wa");
	if(!f_out)
	{
		perror("File opening failed.");
		return -1;
	}
	ip_dump(f_out);

	return 0;
}
