/**
 * @file my_pregel_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef MY_PREGEL_POSTAMBLE_H_INCLUDED
#define MY_PREGEL_POSTAMBLE_H_INCLUDED

#ifdef IP_USE_SPREAD
	#ifdef IP_USE_SINGLE_BROADCAST
		#include "combiner_spread_single_broadcast_postamble.h"
	#else // ifndef IP_USE_SINGLE_BROADCAST
		#include "combiner_spread_postamble.h"
	#endif // if(n)def IP_USE_SINGLE_BROADCAST
#else // ifndef IP_USE_SPREAD
	#ifdef IP_USE_SINGLE_BROADCAST
		#include "combiner_single_broadcast_postamble.h"
	#else // ifndef IP_USE_SINGLE_BROADCAST
		#include "combiner_postamble.h"
	#endif // if(n)def IP_USE_SINGLE_BROADCAST
#endif // if(n)def IP_USE_SPREAD

size_t ip_get_superstep()
{
	return ip_superstep;
}

void ip_increment_superstep()
{
	ip_superstep++;
}

bool ip_is_first_superstep()
{
	return ip_get_superstep() == 0;
}

void ip_reset_superstep()
{
	ip_superstep = 0;
}

size_t ip_get_meta_superstep()
{
	return ip_meta_superstep;
}

void ip_increment_meta_superstep()
{
	ip_meta_superstep++;
}

bool ip_is_first_meta_superstep()
{
	return ip_get_meta_superstep() == 0;
}

size_t ip_get_meta_superstep_count()
{
	return ip_meta_superstep_count;
}

void ip_set_meta_superstep_count(size_t meta_superstep_count)
{
	ip_meta_superstep_count = meta_superstep_count;
}

void ip_set_vertices_count(size_t vertices_count)
{
	ip_vertices_count = vertices_count;
}

size_t ip_get_vertices_count()
{
	return ip_vertices_count;
}

struct ip_vertex_t* ip_get_vertex_by_location(size_t location)
{
	return &ip_all_vertices[location];
}

struct ip_vertex_t* ip_get_vertex_by_id(IP_VERTEX_ID_TYPE id)
{
	#if (defined(IP_ID_OFFSET) && IP_MINIMUM_ID == 0) \
	  || defined(FORCE_DIRECT_MAPPING)
		/* Either there is no offset, either there is an offset but we don't
		mind wasting these offset elements. For exaiple, a graph where the
		minimum ID is 3 means that elements [0],[1] and [2] will be unused. With
		a force mapping, these elements will be left unused so there will be 
		some memory wasted, but it is up to the user to make that decision: 
		extra computation vs wasted memory.	*/
		return ip_get_vertex_by_location(id);
	#else
		/* There is an offset, and we do not want to ignore it. Therefore, we
		must take this offset in consideration when selecting the index of the
		array element corresponding to a given vertex identifier. */
		size_t location = id - IP_ID_OFFSET;
		return ip_get_vertex_by_location(location);
	#endif
}

void ip_dump(FILE* f)
{
	double timer_dump_start = omp_get_wtime();
	double timer_dump_stop = 0;
	unsigned char progress = 0;
	size_t i = 0;
	size_t chunk = ip_get_vertices_count() / 100;

	if(chunk == 0)
	{
		chunk = 1;
	}
	printf("%3u %% vertices stored.\r", progress);
	fflush(stdout);
	while(i < ip_get_vertices_count())
	{
		ip_serialise_vertex(f, ip_get_vertex_by_location(i));
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
	printf("Duiping finished in %fs.\n", timer_dump_stop - timer_dump_start);
}

void* ip_safe_malloc(size_t size_to_malloc)
{
	void* ptr = malloc(size_to_malloc);
	if(ptr == NULL)
	{
		exit(-1);
	}
	return ptr;
}

void* ip_safe_realloc(void* ptr, size_t size_to_realloc)
{
	ptr = realloc(ptr, size_to_realloc);
	if(ptr == NULL)
	{
		exit(-1);
	}
	return ptr;
}

void ip_safe_free(void* ptr)
{
	if(ptr != NULL)
	{
		free(ptr);
		ptr = NULL;
	}
}

void ip_safe_fread(void * ptr, size_t size, size_t count, FILE * stream)
{
	if(fread(ptr, size, count, stream) != count)
	{
		printf("Failed to read.\n");
		exit(-1);
	}
}

void ip_safe_fwrite(void * ptr, size_t size, size_t count, FILE * stream)
{
	if(fwrite(ptr, size, count, stream) != count)
	{
		printf("Failed to write.'\n");
		exit(-1);
	}
}

#endif // MY_PREGEL_POSTAMBLE_H_INCLUDED
