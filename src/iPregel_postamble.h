/**
 * @file iPregel_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef MY_PREGEL_POSTAMBLE_H_INCLUDED
#define MY_PREGEL_POSTAMBLE_H_INCLUDED

#include <string.h>

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
	#if (defined(IP_ID_OFFSET) && IP_ID_OFFSET == 0) \
	  || defined(IP_FORCE_DIRECT_MAPPING)
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
		printf("Failed to allocate %zu bytes.\n", size_to_malloc);
		exit(-1);
	}
	return ptr;
}

void* ip_safe_realloc(void* ptr, size_t size_to_realloc)
{
	ptr = realloc(ptr, size_to_realloc);
	if(ptr == NULL)
	{
		printf("Failed to reallocate to %zu bytes.\n", size_to_realloc);
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

FILE* ip_safe_fopen(const char* file_path, const char* mode)
{
	FILE* f = fopen(file_path, mode);
	if(f == NULL)
	{
		printf("Cannot open the file \"%s\" in mode \"%s\".\n", file_path, mode);
		exit(-1);
	}
	return f;
}
void ip_safe_fread(void * ptr, size_t size, size_t count, FILE * stream)
{
	if(fread(ptr, size, count, stream) != count)
	{
		printf("Failed to read %zu elements of %zu bytes each.\n", count, size);
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

void ip_init(const char* file_path, int number_of_threads, bool directed, bool weighted)
{
	double timer_init_start = omp_get_wtime();
	double timer_init_stop = 0; 

	// Initialise OpenMP variable
	omp_set_num_threads(number_of_threads);
	#pragma omp parallel
	{
		#pragma omp master
		{
			printf("[INFO] Using %d OpenMP threads.\n", omp_get_num_threads());
		}
	}

	// Load the graph
	ip_load_graph(file_path, directed, weighted);
		
	timer_init_stop = omp_get_wtime();
	printf("[TIMING] Initialisation finished in %fs.\n", timer_init_stop - timer_init_start);
}

void ip_load_graph(const char* file_path, bool directed, bool weighted)
{
	if(directed)
	{
		printf("[INFO] The application indicates that the graph must be directed, so the graph passed is expected to be so.\n");
	}
	else
	{
		printf("[INFO] The application indicates that the graph must be undirected, so the graph passed is expected to be so.\n");
	}

	if(weighted)
	{
		printf("[INFO] The application indicates that the graph must be weighted, so the graph passed is expected to be so.\n");
	}
	else
	{
		printf("[INFO] The application indicates that the graph must not be weighted, so the graph passed is expected to be so.\n");
	}

	printf("[INFO] Starting graph loading.\n");

	// Open config file to get number of vertices and edges
	char config_file_extension[] = ".config";
	char config_file_name[strlen(file_path) + strlen(config_file_extension) + 1];
	memcpy(config_file_name, file_path, sizeof(char) * strlen(file_path));
	memcpy(config_file_name + strlen(file_path), config_file_extension, sizeof(char) * strlen(config_file_extension));
	config_file_name[strlen(file_path) + strlen(config_file_extension)] = '\0';
	printf("\t- Loading configuration file from: \"%s\".\n", config_file_name);
	FILE* config_file = ip_safe_fopen(config_file_name, "r");
	if(fscanf(config_file, "%zu %zu", &ip_vertices_count, &ip_edges_count) != 2)
	{
		printf("\t- Failure in reading the number of vertices and edges. Abort...\n");
		fclose(config_file);
		exit(-1);
	}
	fclose(config_file);
	printf("\t\t- %zu vertices\n\t\t- %zu edges\n", ip_vertices_count, ip_edges_count);
	
	// Allocate vertices
	ip_active_vertices = ip_get_vertices_count();
	ip_all_vertices = (struct ip_vertex_t*)ip_safe_malloc(sizeof(struct ip_vertex_t) * ip_get_vertices_count());
	
	// The number of vertices and edges are known, the vertices are allocated so tell whatever version used to launch its own initialisation.
	ip_init_specific();

	// Initialise vertices
	printf("\t- Initialising vertices\n");
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");
	printf("\t\t| THREAD ID | FIRST VERTEX |  LAST VERTEX |    #VERTICES | %%VERTICES |\n");
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");
	#pragma omp parallel for default(none) shared(ip_all_vertices, ip_vertices_count)
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		bool i_am_last_thread = omp_get_thread_num() == omp_get_num_threads() - 1;
		IP_VERTEX_ID_TYPE vertex_chunk = (ip_get_vertices_count() - (ip_get_vertices_count() % omp_get_num_threads())) / omp_get_num_threads();
		IP_VERTEX_ID_TYPE vertex_start = vertex_chunk * omp_get_thread_num();
		if(i_am_last_thread) { vertex_chunk += ip_get_vertices_count() % omp_get_num_threads(); } // Must be AFTER vertex_start
		printf("\t\t| %9d | %12u | %12u | %12u | %9.5f |\n", omp_get_thread_num(), vertex_start, vertex_start + vertex_chunk - 1, vertex_chunk, ((float)vertex_chunk) / ((float)ip_get_vertices_count()) * 100.0f);
		ip_init_vertex_range(vertex_start, vertex_start + vertex_chunk - 1);
	}
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");

	// Open offset file and load them in parallel
	char offset_file_extension[] = ".idx";
	char offset_file_name[strlen(file_path) + strlen(offset_file_extension) + 1];
	memcpy(offset_file_name, file_path, sizeof(char) * strlen(file_path));
	memcpy(offset_file_name + strlen(file_path), offset_file_extension, sizeof(char) * strlen(offset_file_extension));
	offset_file_name[strlen(file_path) + strlen(offset_file_extension)] = '\0';
	printf("\t- Loading offset file from: \"%s\".\n", offset_file_name);
	IP_VERTEX_ID_TYPE* ip_all_offsets = (IP_VERTEX_ID_TYPE*)ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * ip_vertices_count); 
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");
	printf("\t\t| THREAD ID | FIRST OFFSET |  LAST OFFSET |     #OFFSETS |  %%OFFSETS |\n");
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");
	#pragma omp parallel for default(none) shared(offset_file_name, ip_all_offsets) firstprivate(ip_vertices_count)
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		bool i_am_last_thread = omp_get_thread_num() == (omp_get_num_threads() - 1);
		IP_VERTEX_ID_TYPE offset_chunk = (ip_get_vertices_count() - (ip_get_vertices_count() % omp_get_num_threads())) / omp_get_num_threads();
		IP_VERTEX_ID_TYPE offset_start = offset_chunk * omp_get_thread_num();
		if(i_am_last_thread) { offset_chunk += ip_get_vertices_count() % omp_get_num_threads(); } // Must be AFTER vertex_start
		FILE* offset_file = ip_safe_fopen(offset_file_name, "rb");
		fseek(offset_file, offset_start * sizeof(IP_VERTEX_ID_TYPE), SEEK_SET);
		printf("\t\t| %9d | %12u | %12u | %12u | %9.5f |\n", omp_get_thread_num(), offset_start, offset_start + offset_chunk - 1, offset_chunk, ((float)offset_chunk) * 100.0f / ((float)ip_vertices_count));
		ip_safe_fread(&ip_all_offsets[offset_start], sizeof(IP_VERTEX_ID_TYPE), offset_chunk, offset_file);
		// No need to use the offset file anymore since it's now loaded in memory.
		fclose(offset_file);
	}
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");

	// Open adjacency file and load out neighbours in parallel
	char adjacency_file_extension[] = ".adj";
	char adjacency_file_name[strlen(file_path) + strlen(adjacency_file_extension) + 1];
	memcpy(adjacency_file_name, file_path, sizeof(char) * strlen(file_path));
	memcpy(adjacency_file_name + strlen(file_path), adjacency_file_extension, sizeof(char) * strlen(adjacency_file_extension));
	adjacency_file_name[strlen(file_path) + strlen(adjacency_file_extension)] = '\0';
	printf("\t- Loading adjacency file from: \"%s\".\n", adjacency_file_name);
	IP_VERTEX_ID_TYPE* ip_all_out_neighbours = (IP_VERTEX_ID_TYPE*)ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * ip_edges_count);
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");
	printf("\t\t| THREAD ID |   FIRST EDGE |    LAST EDGE |       #EDGES |    %%EDGES |\n");
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");
	#pragma omp parallel for default(none) shared(ip_all_out_neighbours, ip_all_offsets) firstprivate(ip_edges_count, adjacency_file_name, directed)
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		bool i_am_first_thread = omp_get_thread_num() == 0;
		bool i_am_last_thread = omp_get_thread_num() == (omp_get_num_threads() - 1);
		IP_VERTEX_ID_TYPE vertex_chunk = (ip_get_vertices_count() - (ip_get_vertices_count() % omp_get_num_threads())) / omp_get_num_threads();
		IP_VERTEX_ID_TYPE vertex_start = vertex_chunk * omp_get_thread_num();
		if(i_am_last_thread) { vertex_chunk += ip_get_vertices_count() % omp_get_num_threads(); } // Must be AFTER vertex_start
		// Vertex_end is the first vertex that NO LONGER belongs to use (like std::vector::end()).
		// Do not replace vertex_end with the hardcoded vertex_start + vertex_chunk because if we are the first thread we are going to update out vertex_start but we want out vertex_end to remain the same. By using evaluating "vertex_start + vertex_chunk" we find something that is of course different than what it was equal to before we modify vertex_start.
		IP_VERTEX_ID_TYPE vertex_end = vertex_start + vertex_chunk;
		IP_VERTEX_ID_TYPE edge_start = ip_all_offsets[vertex_start];
		// Edge_end is the first edge that NO LONGER belongs to us (like std::vector::end()).
		IP_VERTEX_ID_TYPE edge_end = i_am_last_thread ? ip_edges_count : ip_all_offsets[vertex_start + vertex_chunk];
		IP_VERTEX_ID_TYPE edge_chunk = edge_end - edge_start;
		printf("\t\t| %9d | %12u | %12u | %12u | %9.5f |\n", omp_get_thread_num(), edge_start, edge_start + edge_chunk - 1, edge_chunk, ((float)edge_chunk) * 100.0f / ((float)ip_edges_count));
		// Go to my first edge and read my chunk
		FILE* adjacency_file = ip_safe_fopen(adjacency_file_name, "rb");
		fseek(adjacency_file, edge_start * sizeof(IP_VERTEX_ID_TYPE), SEEK_SET);
		ip_safe_fread(&ip_all_out_neighbours[edge_start], sizeof(IP_VERTEX_ID_TYPE), edge_chunk, adjacency_file);
		// Now that edges are loaded in memory, the file is no longer needed.
		fclose(adjacency_file);
		// If the framework needs the out-neighbours, we connect the out-neighbours that we just loaded to their source vertex.
		IP_VERTEX_ID_TYPE j = vertex_start;
		if(i_am_first_thread)
		{
			#ifdef IP_NEEDS_OUT_NEIGHBOUR_IDS
				ip_get_vertex_by_location(vertex_start)->out_neighbours = &ip_all_out_neighbours[0];
			#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_IDS
			#ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
				if(!directed)
				{
					ip_get_vertex_by_location(vertex_start)->in_neighbours = &ip_all_out_neighbours[0];
				}
			#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_IDS

			vertex_start++;
		}
		for(j = vertex_start; j < vertex_end; j++)
		{
			#ifdef IP_NEEDS_OUT_NEIGHBOUR_IDS
				ip_get_vertex_by_location(j)->out_neighbours = &ip_all_out_neighbours[ip_all_offsets[j]];
			#endif // ifdef IP_UNUSED_OUT_NEIGHBOUR_IDS
			#ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
				if(!directed)
				{
					ip_get_vertex_by_location(j)->in_neighbours = &ip_all_out_neighbours[ip_all_offsets[j]];
				}
			#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
			#ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
				ip_get_vertex_by_location(j-1)->out_neighbour_count = ip_all_offsets[j] - ip_all_offsets[j-1];
			#endif // IP_NEEDS_OUT_NEIGHBOUR_COUNT
			#ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
				if(!directed)
				{
					ip_get_vertex_by_location(j-1)->in_neighbour_count = ip_all_offsets[j] - ip_all_offsets[j-1];
				}
			#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
		}
		#ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
			if(i_am_last_thread)
			{
				ip_get_vertex_by_location(j-1)->out_neighbour_count = ip_edges_count - ip_all_offsets[j-1];
			}
			else
			{
				ip_get_vertex_by_location(j-1)->out_neighbour_count = ip_all_offsets[j] - ip_all_offsets[j-1];
			}
		#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
		#ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
			if(!directed)
			{
				if(i_am_last_thread)
				{
					ip_get_vertex_by_location(j-1)->in_neighbour_count = ip_edges_count - ip_all_offsets[j-1];
				}
				else
				{
					ip_get_vertex_by_location(j-1)->in_neighbour_count = ip_all_offsets[j] - ip_all_offsets[j-1];
				}
			}
		#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
	}
	printf("\t\t+-----------+--------------+--------------+--------------+-----------+\n");

	printf("\t- Mirror in-neighbours\n");
	if(!directed)
	{
		printf("\t\t- Already done since the graph is undirected.\n");
	}
	else
	{
		#if defined(IP_NEEDS_IN_NEIGHBOUR_IDS) || defined(IP_NEEDS_IN_NEIGHBOURS_COUNT)
			size_t total_in_neighbours = 0;
			// If in-neighbours are needed, handle them 
			#pragma omp parallel for default(none) shared(ip_all_offsets, ip_all_out_neighbours) firstprivate(ip_edges_count) reduction(+:total_in_neighbours)
			for(int i = 0; i < omp_get_num_threads(); i++)
			{
				bool i_am_last_thread = omp_get_thread_num() == omp_get_num_threads() - 1;
				IP_VERTEX_ID_TYPE vertex_chunk = (ip_get_vertices_count() - (ip_get_vertices_count() % omp_get_num_threads())) / omp_get_num_threads();
				IP_VERTEX_ID_TYPE vertex_start = vertex_chunk * omp_get_thread_num();
				if(i_am_last_thread) { vertex_chunk += ip_get_vertices_count() % omp_get_num_threads(); } // Must be AFTER vertex_start
				// Vertex_end is the first vertex that NO LONGER belongs to us (like std::vector::end()).
				IP_VERTEX_ID_TYPE vertex_end = vertex_start + vertex_chunk;
				IP_VERTEX_ID_TYPE source = 0;
				size_t offset_limit = ip_all_offsets[1] - 1; // TODO Be careful, if second offset is 0 too, then it will give (0 - 1) which is likely to end up at 2^64 - 1
				for(size_t j = 0; j < ip_edges_count; j++) 
				{
					if(ip_all_out_neighbours[j] >= vertex_start && ip_all_out_neighbours[j] < vertex_end)
					{
						
						struct ip_vertex_t* destination = ip_get_vertex_by_id(ip_all_out_neighbours[j]);
						destination->in_neighbour_count++;
						#ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
							if(destination->in_neighbour_count == 1)
							{
								destination->in_neighbours = (IP_VERTEX_ID_TYPE*)ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * destination->in_neighbour_count);
							}
							else
							{
								destination->in_neighbours = (IP_VERTEX_ID_TYPE*)ip_safe_realloc(destination->in_neighbours, sizeof(IP_VERTEX_ID_TYPE) * destination->in_neighbour_count);
							}
							destination->in_neighbours[destination->in_neighbour_count-1] = source;
						#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
						total_in_neighbours++;
					}
					while(source < ip_get_vertices_count() && j == offset_limit) // In case a vertex has no out neighbours, its offset will be the same as the following one so we want to go the following one directly.
					{
						source++;
						offset_limit = ip_all_offsets[source+1] - 1;
					}
				}
			}
			printf("\t\t- %zu in neighbours created.\n", total_in_neighbours);
			if(total_in_neighbours == ip_edges_count)
			{
				printf("\t\t- Matches the number of out-neighbours.\n");
			}
			else
			{
				printf("\t\t- Different from the number of out-neighbours. There is a bug in the in-neighbour mirroring.\n");
				exit(-1);
			}
		#endif // if defined(IP_NEEDS_OUT_NEIGHBOUR_IDS) || defined(IP_NEEDS_OUT_NEIGHBOUR_COUNT)
	}

	//////////
	// TODO //
	//////////
	// Check that offsets are read and manipulated as long because the number of edges may be far beyond the maximum value encodable on the type used to encore vertex identifiers.

	printf("\t- Memory freeing\n");
	if(directed)
	{
		#ifndef IP_NEEDS_OUT_NEIGHBOUR_IDS
			printf("\t\t- Out neighbour identifiers: %zu bytes freed.\n", ip_edges_count * sizeof(IP_VERTEX_ID_TYPE));
			free(ip_all_out_neighbours);
		#endif // ifndef IP_NEEDS_OUT_NEIGHBOUR_IDS
		#ifndef IP_NEEDS_OUT_NEIGHBOUR_COUNT
			printf("\t\t- Offsets loaded from offset file %s: %zu bytes saved.\n", offset_file_name, ip_get_vertices_count() * sizeof(IP_VERTEX_ID_TYPE)); 
			free(ip_all_offsets);
		#endif // ifndef IP_NEEDS_OUT_NEIGHBOUR_COUNT
	}
}

#endif // MY_PREGEL_POSTAMBLE_H_INCLUDED
