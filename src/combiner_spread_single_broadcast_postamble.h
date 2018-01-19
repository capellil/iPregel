/**
 * @file combiner_spread_single_broadcast_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_SPREAD_SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
#define COMBINER_SPREAD_SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED

#include <omp.h>
#include <string.h>

void mp_add_target(MP_VERTEX_ID_TYPE id)
{
	if(mp_all_targets.size == mp_all_targets.max_size)
	{
		mp_all_targets.max_size++;
		mp_all_targets.data = mp_safe_realloc(mp_all_targets.data, sizeof(MP_VERTEX_ID_TYPE) * mp_all_targets.max_size);
	}

	mp_all_targets.data[mp_all_targets.size] = id;
	mp_all_targets.size++;
}

bool mp_has_message(struct mp_vertex_t* v)
{
	return v->has_message;
}

bool mp_get_next_message(struct mp_vertex_t* v, MP_MESSAGE_TYPE* message_value)
{
	if(v->has_message)
	{
		*message_value = v->message;
		v->has_message = false;
		mp_messages_left_omp[omp_get_thread_num()]++;
		return true;
	}
	return false;
}

void mp_send_message(MP_VERTEX_ID_TYPE id, MP_MESSAGE_TYPE message)
{
	(void)(id);
	(void)(message);
	printf("The function send_message should not be used in the SINGLE_BROADCAST \
version; only broadcast() should be called, and once per superstep maximum.\n");
	exit(-1);
}

void mp_broadcast(struct mp_vertex_t* v, MP_MESSAGE_TYPE message)
{
	v->has_broadcast_message = true;
	v->broadcast_message = message;
	for(MP_NEIGHBOURS_COUNT_TYPE i = 0; i < v->out_neighbours_count; i++)
	{
		/* Should use "#pragma omp atomic write" to protect the data race, but
		 * since all threads would race to put the same value in the variable,
		 * it has been purposely left unprotected.
		 */
		mp_get_vertex_by_id(v->out_neighbours[i])->broadcast_target = true;
	}
}

void mp_fetch_broadcast_messages(struct mp_vertex_t* v)
{
	MP_NEIGHBOURS_COUNT_TYPE i = 0;
	while(i < v->in_neighbours_count && !mp_get_vertex_by_id(v->in_neighbours[i])->has_broadcast_message)
	{
		i++;
	}

	if(i >= v->in_neighbours_count)
	{
		v->has_message = false;
	}
	else
	{
		mp_messages_left_omp[omp_get_thread_num()]++;
		v->has_message = true;
		v->message = mp_get_vertex_by_id(v->in_neighbours[i])->broadcast_message;
		i++;
		MP_VERTEX_ID_TYPE spread_neighbour_id;
		struct mp_vertex_t* temp_vertex = NULL;
		while(i < v->in_neighbours_count)
		{
			spread_neighbour_id = v->in_neighbours[i];
			temp_vertex = mp_get_vertex_by_id(spread_neighbour_id);
			if(temp_vertex->has_broadcast_message)
			{
				mp_combine(&v->message, temp_vertex->broadcast_message);
			}
			i++;
		}
	}	
}

void mp_add_edge(MP_VERTEX_ID_TYPE src, MP_VERTEX_ID_TYPE dest)
{
	struct mp_vertex_t* v;

	//////////////////////////////
	// Add the dest to the src //
	////////////////////////////
	v = mp_get_vertex_by_id(src);
	v->id = src;
	v->out_neighbours_count++;
	if(v->out_neighbours_count == 1)
	{
		v->out_neighbours = mp_safe_malloc(sizeof(MP_VERTEX_ID_TYPE));
	}
	else
	{
		v->out_neighbours = mp_safe_realloc(v->out_neighbours, sizeof(MP_VERTEX_ID_TYPE) * v->out_neighbours_count);
	}
	v->out_neighbours[v->out_neighbours_count-1] = dest;
	
	//////////////////////////////
	// Add the src to the dest //
	////////////////////////////
	v = mp_get_vertex_by_id(dest);
	v->id = dest;
	v->in_neighbours_count++;
	if(v->in_neighbours_count == 1)
	{
		v->in_neighbours = mp_safe_malloc(sizeof(MP_VERTEX_ID_TYPE));
	}
	else
	{
		v->in_neighbours = mp_safe_realloc(v->in_neighbours, sizeof(MP_VERTEX_ID_TYPE) * v->in_neighbours_count);
	}
	v->in_neighbours[v->in_neighbours_count-1] = src;
}

int mp_init(FILE* f, size_t number_of_vertices, size_t number_of_edges)
{
	(void)number_of_edges;
	double timer_init_start = omp_get_wtime();
	double timer_init_stop = 0;
	struct mp_vertex_t* temp_vertex = NULL;

	mp_set_vertices_count(number_of_vertices);
	mp_all_vertices = (struct mp_vertex_t*)mp_safe_malloc(sizeof(struct mp_vertex_t) * mp_get_vertices_count());
	mp_all_targets.max_size = mp_get_vertices_count();
	mp_all_targets.size = mp_get_vertices_count();
	mp_all_targets.data = mp_safe_malloc(sizeof(MP_VERTEX_ID_TYPE) * mp_all_targets.max_size);

	#pragma omp parallel for default(none) private(temp_vertex) shared(mp_all_targets)
	for(size_t i = mp_get_id_offset(); i < mp_get_id_offset() + mp_get_vertices_count(); i++)
	{
		temp_vertex = mp_get_vertex_by_location(i);
		temp_vertex->broadcast_target = false;
		temp_vertex->has_message = false;
		temp_vertex->has_broadcast_message = false;
		mp_all_targets.data[i-1] = i;
	}

	mp_deserialise(f);
	mp_active_vertices = number_of_vertices;

	timer_init_stop = omp_get_wtime();
	printf("Initialisation finished in %fs.\n", timer_init_stop - timer_init_start);
		
	return 0;
}

int mp_run()
{
	double timer_superstep_total = 0;
	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;

	while(mp_get_meta_superstep() < mp_get_meta_superstep_count())
	{
		mp_reset_superstep();
		while(mp_is_first_superstep() || mp_all_targets.size > 0)
		{
			timer_superstep_start = omp_get_wtime();
			#pragma omp parallel default(none) shared(mp_messages_left, \
													  mp_messages_left_omp, \
													  mp_all_targets)
			{
				struct mp_vertex_t* temp_vertex = NULL;

				#pragma omp for
				for(size_t i = 0; i < mp_all_targets.size; i++)
				{
					temp_vertex = mp_get_vertex_by_location(mp_all_targets.data[i]);
					mp_compute(temp_vertex);
				}
			
				// Count how many messages have been consumed by vertices.	
				#pragma omp for reduction(-:mp_messages_left) 
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left -= mp_messages_left_omp[i];
					mp_messages_left_omp[i] = 0;
				}
			
				#pragma omp single
				{
					mp_all_targets.size = 0;
					for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
					{
						temp_vertex = mp_get_vertex_by_location(i);
						if(temp_vertex->broadcast_target)
						{
							mp_add_target(i);
						}
					}
				}
		
				// Get the messages broadcasted by neighbours, but only for those
				// who have neighbours who broadcasted.
				#pragma omp for
				for(size_t i = 0; i < mp_all_targets.size; i++)
				{
					temp_vertex = mp_get_vertex_by_location(mp_all_targets.data[i]);
					if(temp_vertex->broadcast_target)
					{
						mp_fetch_broadcast_messages(temp_vertex);
						temp_vertex->broadcast_target = false;
					}
				}
	
				#pragma omp for
				for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
				{
					mp_get_vertex_by_location(i)->has_broadcast_message = false;
				}
				
				// Count how many vertices have a message.
				#pragma omp for reduction(+:mp_messages_left)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left += mp_messages_left_omp[i];
					mp_messages_left_omp[i] = 0;
				}
			} // End of OpenMP parallel region
	
			timer_superstep_stop = omp_get_wtime();
			timer_superstep_total += timer_superstep_stop - timer_superstep_start;
			printf("Meta-superstep %zu superstep %zu finished in %fs; %zu active vertices and %zu messages left.\n", mp_get_meta_superstep(), mp_get_superstep(), timer_superstep_stop - timer_superstep_start, mp_all_targets.size, mp_messages_left);
			mp_increment_superstep();
		}
		mp_increment_meta_superstep();
	}

	printf("Total time of supersteps: %fs.\n", timer_superstep_total);
	
	return 0;
}

void mp_vote_to_halt(struct mp_vertex_t* v)
{
	(void)(v);
}

#endif // COMBINER_SPREAD_SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
