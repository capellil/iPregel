/**
 * @file my_pregel_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef MY_PREGEL_POSTAMBLE_H_INCLUDED
#define MY_PREGEL_POSTAMBLE_H_INCLUDED

#ifdef USE_COMBINER
	#ifdef USE_SPREAD
		#ifdef USE_SINGLE_BROADCAST
			#include "combiner_spread_single_broadcast_postamble.h"
		#else // ifndef USE_SINGLE_BROADCAST
			#include "combiner_spread_postamble.h"
		#endif // if(n)def USE_SINGLE_BROADCAST
	#else // ifndef USE_SPREAD
		#ifdef USE_SINGLE_BROADCAST
			#include "combiner_single_broadcast_postamble.h"
		#else // ifndef USE_SINGLE_BROADCAST
			#include "combiner_postamble.h"
		#endif // if(n)def USE_SINGLE_BROADCAST
	#endif // if(n)def USE_SPREAD
#else // ifndef USE_COMBINER
	#include "no_combiner_postamble.h"	
#endif // if(n)def USE_COMBINER

void dump(FILE* f)
{
	double timer_dump_start = omp_get_wtime();
	double timer_dump_stop = 0;
	
	unsigned int chunk = vertices_count / 100;
	if(chunk == 0)
	{
		chunk = 1;
	}
	unsigned int progress = 0;
	unsigned int i = 0;
	printf("%3u %% vertices stored.\r", progress);
	fflush(stdout);
	i++;
	while(i < vertices_count)
	{
		serialise_vertex(f, &all_vertices[i]);
		if(i % chunk == 0)
		{
			progress++;
			printf("%3u %%\r", progress);
			fflush(stdout);
		}
		i++;
	}
	printf("100 %%\n");

	timer_dump_stop = omp_get_wtime();
	printf("Dumping finished in %fs.\n", timer_dump_stop - timer_dump_start);
}

void* safe_malloc(size_t size_to_malloc)
{
	void* ptr = malloc(size_to_malloc);
	if(ptr == NULL)
	{
		exit(-1);
	}
	return ptr;
}

void* safe_realloc(void* ptr, size_t size_to_realloc)
{
	ptr = realloc(ptr, size_to_realloc);
	if(ptr == NULL)
	{
		exit(-1);
	}
	return ptr;
}

void safe_free(void* ptr)
{
	if(ptr != NULL)
	{
		free(ptr);
		ptr = NULL;
	}
}

#endif // MY_PREGEL_POSTAMBLE_H_INCLUDED
