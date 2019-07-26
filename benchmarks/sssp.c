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

typedef unsigned int IP_VERTEX_ID_TYPE;
typedef IP_VERTEX_ID_TYPE IP_NEIGHBOUR_COUNT_TYPE;
typedef IP_VERTEX_ID_TYPE IP_MESSAGE_TYPE;
typedef IP_VERTEX_ID_TYPE IP_VALUE_TYPE;
#include "iPregel.h"
// DBLP: minimum vertex id = 0
// liveJournal: minimum vertex id = 0
// Orkut: minimum vertex id = 2
// Friendster: minimum vertex id = 101
const IP_VERTEX_ID_TYPE start_vertex = 2;

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
	(void)(f);
	(void)(v);
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
	bool weighted = true;
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
