/**
 * @file combiner_spread_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
#define COMBINER_SPREAD_POSTAMBLE_H_INCLUDED

#include <omp.h>
#include <string.h>

bool ip_has_message(struct ip_vertex_t* v)
{
	return v->has_message;
}

bool ip_get_next_message(struct ip_vertex_t* v, IP_MESSAGE_TYPE* message_value)
{
	if(v->has_message)
	{
		*message_value = v->message;
		v->has_message = false;
		ip_messages_left_omp[omp_get_thread_num()]--; //TODO Redesign to use incr.
		return true;
	}

	return false;
}

void ip_add_spread_vertex(IP_VERTEX_ID_TYPE id)
{
	struct ip_vertex_list_t* my_list = &ip_all_spread_vertices_omp[omp_get_thread_num()];
	if(my_list->size == my_list->max_size)
	{
		my_list->max_size++;
		my_list->data = ip_safe_realloc(my_list->data, sizeof(IP_VERTEX_ID_TYPE) * my_list->max_size);
	}

	my_list->data[my_list->size] = id;
	my_list->size++;
}

void ip_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE message)
{
	struct ip_vertex_t* temp_vertex = ip_get_vertex_by_id(id);
	ip_lock_acquire(&temp_vertex->lock);
	if(temp_vertex->has_message_next)
	{
		ip_combine(&temp_vertex->message_next, message);
		ip_lock_release(&temp_vertex->lock);
	}
	else
	{
		temp_vertex->has_message_next = true;
		temp_vertex->message_next = message;
		ip_lock_release(&temp_vertex->lock);
		ip_add_spread_vertex(id);
		ip_messages_left_omp[omp_get_thread_num()]++;
	}
}

void ip_broadcast(struct ip_vertex_t* v, IP_MESSAGE_TYPE message)
{
	for(IP_NEIGHBOURS_COUNT_TYPE i = 0; i < v->out_neighbours_count; i++)
	{
		ip_send_message(v->out_neighbours[i], message);
	}
}

#ifdef IP_WEIGHTED_EDGES
	void ip_add_edge(IP_VERTEX_ID_TYPE src, IP_VERTEX_ID_TYPE dest, IP_EDGE_WEIGHT_TYPE weight)
#else // ifndef IP_WEIGHTED_EDGES
	void ip_add_edge(IP_VERTEX_ID_TYPE src, IP_VERTEX_ID_TYPE dest)
#endif // if(n)def IP_WEIGHTED_EDGES
{
	//////////////////////////////
	// Add the dest to the src //
	////////////////////////////
	struct ip_vertex_t* src_vertex;
	src_vertex = ip_get_vertex_by_id(src);
	src_vertex->id = src;
	src_vertex->out_neighbours_count++;
	if(src_vertex->out_neighbours_count == 1)
	{
		src_vertex->out_neighbours = ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE));
		#ifdef IP_WEIGHTED_EDGES
			src_vertex->out_edge_weights = ip_safe_malloc(sizeof(IP_EDGE_WEIGHT_TYPE));
		#endif // ifdef IP_WEIGHTED_EDGES
	}
	else
	{
		src_vertex->out_neighbours = ip_safe_realloc(src_vertex->out_neighbours, sizeof(IP_VERTEX_ID_TYPE) * src_vertex->out_neighbours_count);
		#ifdef IP_WEIGHTED_EDGES
			src_vertex->out_edge_weights = ip_safe_realloc(src_vertex->out_edge_weights, sizeof(IP_EDGE_WEIGHT_TYPE) * src_vertex->out_neighbours_count);
		#endif // ifdef IP_WEIGHTED_EDGES
	}
	src_vertex->out_neighbours[src_vertex->out_neighbours_count-1] = dest;
	#ifdef IP_WEIGHTED_EDGES
		src_vertex->out_edge_weights[src_vertex->out_neighbours_count-1] = weight;
	#endif // ifdef IP_WEIGHTED_EDGES
	
	//////////////////////////////
	// Add the src to the dest //
	////////////////////////////
	struct ip_vertex_t* dest_vertex;
	dest_vertex = ip_get_vertex_by_id(dest);
	dest_vertex->id = dest;
	#ifndef IP_UNUSED_IN_NEIGHBOURS
		dest_vertex->in_neighbours_count++;
		#ifndef IP_UNUSED_IN_NEIGHBOUR_IDS
			if(dest_vertex->in_neighbours_count == 1)
			{
				dest_vertex->in_neighbours = ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE));
				#ifdef IP_WEIGHTED_EDGES
					dest_vertex->in_edge_weights = ip_safe_malloc(sizeof(IP_EDGE_WEIGHT_TYPE));
				#endif // ifdef IP_WEIGHTED_EDGES
			}
			else
			{
				dest_vertex->in_neighbours = ip_safe_realloc(dest_vertex->in_neighbours, sizeof(IP_VERTEX_ID_TYPE) * dest_vertex->in_neighbours_count);
				#ifdef IP_WEIGHTED_EDGES
					dest_vertex->in_edge_weights = ip_safe_realloc(dest_vertex->in_edge_weights, sizeof(IP_EDGE_WEIGHT_TYPE) * dest_vertex->in_neighbours_count);
				#endif // ifdef IP_WEIGHTED_EDGES
			}
			dest_vertex->in_neighbours[dest_vertex->in_neighbours_count-1] = src;
			#ifdef IP_WEIGHTED_EDGES
				dest_vertex->in_edge_weights[dest_vertex->in_neighbours_count-1] = weight;
			#endif // ifdef IP_WEIGHTED_EDGES
		#endif // ifndef IP_UNUSED_IN_NEIGHBOUR_IDS
	#endif // ifndef IP_UNUSED_IN_NEIGHBOURS
}

int ip_init(FILE* f, size_t number_of_vertices, size_t number_of_edges)
{
	(void)number_of_edges;
	double timer_init_start = omp_get_wtime();
	double timer_init_stop = 0;
	struct ip_vertex_t* temp_vertex = NULL;

	ip_set_vertices_count(number_of_vertices);
	ip_all_vertices = (struct ip_vertex_t*)ip_safe_malloc(sizeof(struct ip_vertex_t) * ip_get_vertices_count());

	ip_all_spread_vertices.max_size = 1;
	ip_all_spread_vertices.size = 0;
	ip_all_spread_vertices.data = ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * ip_all_spread_vertices.max_size);
	for(int i = 0; i < OMP_NUM_THREADS; i++)
	{
		ip_all_spread_vertices_omp[i].max_size = 1;
		ip_all_spread_vertices_omp[i].size = 0;
		ip_all_spread_vertices_omp[i].data = ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * ip_all_spread_vertices_omp[i].max_size);
	}

	#pragma omp parallel for default(none) private(temp_vertex)
	for(size_t i = IP_ID_OFFSET; i < IP_ID_OFFSET + ip_get_vertices_count(); i++)
	{
		temp_vertex = ip_get_vertex_by_location(i);
		temp_vertex->has_message = false;
		temp_vertex->has_message_next = false;
		temp_vertex->out_neighbours_count = 0;
		temp_vertex->out_neighbours = NULL;
		#ifdef IP_WEIGHTED_EDGES
			temp_vertex->out_edge_weights = NULL;
		#endif
		#ifndef IP_UNUSED_IN_NEIGHBOURS
			temp_vertex->in_neighbours_count = 0;
			#ifndef IP_UNUSED_IN_NEIGHBOUR_IDS
				temp_vertex->in_neighbours = NULL;
			#endif
			#ifdef IP_WEIGHTED_EDGES
				temp_vertex->in_edge_weights = NULL;
			#endif
		#endif
		ip_lock_init(&temp_vertex->lock);
	}

	ip_deserialise(f);

	timer_init_stop = omp_get_wtime();
	printf("Initialisation finished in %fs.\n", timer_init_stop - timer_init_start);
		
	return 0;
}

int ip_run()
{
	double timer_superstep_total = 0;
	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;

	while(ip_get_meta_superstep() < ip_get_meta_superstep_count())
	{
		ip_reset_superstep();
		while(ip_is_first_superstep() || ip_spread_vertices_count > 0)
		{
			ip_spread_vertices_count = 0;
			timer_superstep_start = omp_get_wtime();
			#pragma omp parallel default(none) shared(ip_messages_left, \
													  ip_messages_left_omp, \
													  ip_spread_vertices_count, \
													  ip_all_spread_vertices, \
													  ip_all_spread_vertices_omp)
			{
				struct ip_vertex_t* temp_vertex = NULL;

				if(ip_is_first_superstep())
				{
					#pragma omp for
					for(size_t i = IP_ID_OFFSET; i < ip_get_vertices_count() + IP_ID_OFFSET; i++)
					{
						temp_vertex = ip_get_vertex_by_location(i);
						ip_compute(temp_vertex);
					}
				}
				else
				{
					IP_VERTEX_ID_TYPE spread_neighbour_id;
					#pragma omp for
					for(size_t i = 0; i < ip_all_spread_vertices.size; i++)
					{
						spread_neighbour_id = ip_all_spread_vertices.data[i];
						temp_vertex = ip_get_vertex_by_id(spread_neighbour_id);
						ip_compute(temp_vertex);
					}
				}
				
				#pragma omp for reduction(+:ip_messages_left) reduction(+:ip_spread_vertices_count)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					ip_messages_left += ip_messages_left_omp[i];
					ip_messages_left_omp[i] = 0;
					ip_spread_vertices_count += ip_all_spread_vertices_omp[i].size;
				}
				
				#pragma omp single
				{
					if(ip_all_spread_vertices.max_size < ip_spread_vertices_count)
					{
						ip_all_spread_vertices.data = ip_safe_realloc(ip_all_spread_vertices.data, sizeof(IP_VERTEX_ID_TYPE) * ip_spread_vertices_count);
						ip_all_spread_vertices.max_size = ip_spread_vertices_count;
					}
				
					ip_all_spread_vertices.size = 0;
		
					for(int i = 0; i < OMP_NUM_THREADS; i++)
					{
						if(ip_all_spread_vertices_omp[i].size > 0)
						{
							memmove(&ip_all_spread_vertices.data[ip_all_spread_vertices.size], ip_all_spread_vertices_omp[i].data, ip_all_spread_vertices_omp[i].size * sizeof(IP_VERTEX_ID_TYPE));
							ip_all_spread_vertices.size += ip_all_spread_vertices_omp[i].size;
							ip_all_spread_vertices_omp[i].size = 0;
						}
					}
				}
	
				IP_VERTEX_ID_TYPE spread_vertex_id;
				// Take in account only the vertices that have been flagged as
				// spread -> that is, vertices having received a new message.
				#pragma omp for
				for(size_t i = 0; i < ip_all_spread_vertices.size; i++)
				{
					spread_vertex_id = ip_all_spread_vertices.data[i];
					temp_vertex = ip_get_vertex_by_id(spread_vertex_id);
					temp_vertex->has_message = true;
					temp_vertex->message = temp_vertex->message_next;
					temp_vertex->has_message_next = false;
				}
				
				#pragma omp for reduction(+:ip_messages_left)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					ip_messages_left += ip_messages_left_omp[i];
					ip_messages_left_omp[i] = 0;
				}
			} // End of OpenMP parallel region
	
			timer_superstep_stop = omp_get_wtime();
			timer_superstep_total += (timer_superstep_stop - timer_superstep_start);
			printf("Meta-superstep %zu superstep %zu finished in %fs; %zu active vertices and %zu messages left.\n", ip_get_meta_superstep(), ip_get_superstep(), timer_superstep_stop - timer_superstep_start, ip_spread_vertices_count, ip_messages_left);
			ip_increment_superstep();
		}
		ip_increment_meta_superstep();	
	}

	printf("Total time of supersteps: %fs.\n", timer_superstep_total);

	// Free and clean program.	
	#pragma omp for
	for(int i = 0; i < OMP_NUM_THREADS; i++)
	{
		ip_safe_free(ip_all_spread_vertices_omp[i].data);
	}
	ip_safe_free(ip_all_spread_vertices.data);

	return 0;
}

void ip_vote_to_halt(struct ip_vertex_t* v)
{
	(void)(v);
}

#endif // COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
