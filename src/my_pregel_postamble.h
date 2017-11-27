/**
 * @file my_pregel_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef MY_PREGEL_POSTAMBLE_H_INCLUDED
#define MY_PREGEL_POSTAMBLE_H_INCLUDED

#ifdef USE_COMBINER
	#ifdef USE_SINGLE_BROADCAST
		#include "combiner_single_broadcast_postamble.h"
	#else // ifndef USE_SINGLE_BROADCAST
		#ifdef USE_SPREAD
			#include "combiner_spread_postamble.h"
		#else // ifndef USE_SPREAD
			#include "combiner_postamble.h"
		#endif // if(n)def USE_SPREAD
	#endif // if(n)def USE_SINGLE_BROADCAST
#else // ifndef USE_COMBINER
	#include "no_combiner_postamble.h"	
#endif // if(n)def USE_COMBINER

void dump(FILE* f)
{
	for(unsigned int i = 0; i < vertices_count; i++)
	{
		serialise_vertex(f, &all_vertices[i]);
	}
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
