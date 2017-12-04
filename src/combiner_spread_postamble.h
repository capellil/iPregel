/**
 * @file combiner_spread_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
#define COMBINER_SPREAD_POSTAMBLE_H_INCLUDED

#include <omp.h>
#include <string.h>

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
		mp_messages_left_omp[omp_get_thread_num()]--; //TODO Redesign to use incr.
		return true;
	}

	return false;
}

void mp_add_spread_vertex(MP_VERTEX_ID_TYPE id)
{
	struct mp_vertex_list_t* my_list = &mp_all_spread_vertices_omp[omp_get_thread_num()];
	if(my_list->size == my_list->max_size)
	{
		my_list->max_size++;
		my_list->data = mp_safe_realloc(my_list->data, sizeof(MP_VERTEX_ID_TYPE) * my_list->max_size);
	}

	my_list->data[my_list->size] = id;
	my_list->size++;
}

void mp_send_message(MP_VERTEX_ID_TYPE id, MP_MESSAGE_TYPE message)
{
	struct mp_vertex_t* temp_vertex = mp_get_vertex_by_id(id);
	MP_LOCK(&temp_vertex->lock);
	if(temp_vertex->has_message_next)
	{
		mp_combine(&temp_vertex->message_next, message);
		MP_UNLOCK(&temp_vertex->lock);
	}
	else
	{
		temp_vertex->has_message_next = true;
		temp_vertex->active = true;
		temp_vertex->message_next = message;
		MP_UNLOCK(&temp_vertex->lock);
		mp_add_spread_vertex(id);
		mp_messages_left_omp[omp_get_thread_num()]++;
	}
}

void mp_broadcast(struct mp_vertex_t* v, MP_MESSAGE_TYPE message)
{
	for(MP_NEIGHBOURS_COUNT_TYPE i = 0; i < v->out_neighbours_count; i++)
	{
		mp_send_message(v->out_neighbours[i], message);
	}
}

void mp_add_vertex(MP_VERTEX_ID_TYPE id, MP_VERTEX_ID_TYPE* out_neighbours, MP_NEIGHBOURS_COUNT_TYPE out_neighbours_count, MP_VERTEX_ID_TYPE* in_neighbours, MP_NEIGHBOURS_COUNT_TYPE in_neighbours_count)
{
	struct mp_vertex_t* v = mp_get_vertex_by_id(id);
	v->id = id;
	v->out_neighbours_count = out_neighbours_count;
	v->out_neighbours = out_neighbours;
	v->in_neighbours_count = in_neighbours_count;
	v->in_neighbours = in_neighbours;
}

int mp_init(FILE* f, size_t number_of_vertices)
{
	mp_set_vertices_count(number_of_vertices);

	double timer_init_start = omp_get_wtime();
	double timer_init_stop = 0;
	unsigned char progress = 0;
	size_t i = 0;
	size_t chunk = mp_get_vertices_count() / 100;
	struct mp_vertex_t* temp_vertex = NULL;
	
	mp_all_vertices = (struct mp_vertex_t*)mp_safe_malloc(sizeof(struct mp_vertex_t) * mp_get_vertices_count());
	
	mp_all_spread_vertices.max_size = 1;
	mp_all_spread_vertices.size = 0;
	mp_all_spread_vertices.data = mp_safe_malloc(sizeof(MP_VERTEX_ID_TYPE) * mp_all_spread_vertices.max_size);
	for(int i = 0; i < OMP_NUM_THREADS; i++)
	{
		mp_all_spread_vertices_omp[i].max_size = 1;
		mp_all_spread_vertices_omp[i].size = 0;
		mp_all_spread_vertices_omp[i].data = mp_safe_malloc(sizeof(MP_VERTEX_ID_TYPE) * mp_all_spread_vertices_omp[i].max_size);
	}

	if(chunk == 0)
	{
		chunk = 1;
	}
	printf("%3u %% vertices loaded.\r", progress);
	fflush(stdout);
	// Deserialise all the vertices
	while(i < mp_get_vertices_count() && !feof(f))
	{
		mp_deserialise_vertex(f);
		mp_active_vertices++;
		if(i % chunk == 0)
		{
			progress++;
			printf("%3u %%\r", progress);
			fflush(stdout);
		}
		i++;
	}
	printf("100 %%\n");

	#pragma omp parallel for default(none) private(temp_vertex)
	for(i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
	{
		temp_vertex = mp_get_vertex_by_location(i);
		temp_vertex->active = true;
		temp_vertex->has_message = false;
		temp_vertex->has_message_next = false;
		MP_LOCK_INIT(&temp_vertex->lock);
	}
	
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
		while(mp_active_vertices != 0 || mp_messages_left > 0)
		{
			timer_superstep_start = omp_get_wtime();
			mp_active_vertices = 0;
			#pragma omp parallel default(none) shared(mp_active_vertices, \
													  mp_messages_left, \
													  mp_messages_left_omp, \
													  mp_spread_vertices_count, \
													  mp_all_spread_vertices, \
													  mp_all_spread_vertices_omp)
			{
				struct mp_vertex_t* temp_vertex = NULL;

				if(mp_is_first_superstep())
				{
					#pragma omp for reduction(+:mp_active_vertices)
					for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
					{
						temp_vertex = mp_get_vertex_by_location(i);
						if(temp_vertex->active)
						{
							temp_vertex->active = true;
							mp_compute(temp_vertex);
							if(temp_vertex->active)
							{
								mp_active_vertices++;
							}
						}
					}
				}
				else
				{
					MP_VERTEX_ID_TYPE spread_neighbour_id;
					#pragma omp for reduction(+:mp_active_vertices)
					for(size_t i = 0; i < mp_all_spread_vertices.size; i++)
					{
						spread_neighbour_id = mp_all_spread_vertices.data[i];
						temp_vertex = mp_get_vertex_by_id(spread_neighbour_id);
						temp_vertex->active = true;
						mp_compute(temp_vertex);
						if(temp_vertex->active)
						{
							mp_active_vertices++;
						}
					}
				}
				
				#pragma omp for reduction(+:mp_messages_left) reduction(+:mp_spread_vertices_count)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left += mp_messages_left_omp[i];
					mp_messages_left_omp[i] = 0;
					mp_spread_vertices_count += mp_all_spread_vertices_omp[i].size;
				}
				
				#pragma omp single
				{
					if(mp_all_spread_vertices.max_size < mp_spread_vertices_count)
					{
						mp_all_spread_vertices.data = mp_safe_realloc(mp_all_spread_vertices.data, sizeof(MP_VERTEX_ID_TYPE) * mp_spread_vertices_count);
						mp_all_spread_vertices.max_size = mp_spread_vertices_count;
					}
					mp_spread_vertices_count = 0;
				
					mp_all_spread_vertices.size = 0;
		
					for(int i = 0; i < OMP_NUM_THREADS; i++)
					{
						if(mp_all_spread_vertices_omp[i].size > 0)
						{
							memmove(&mp_all_spread_vertices.data[mp_all_spread_vertices.size], mp_all_spread_vertices_omp[i].data, mp_all_spread_vertices_omp[i].size * sizeof(MP_VERTEX_ID_TYPE));
							mp_all_spread_vertices.size += mp_all_spread_vertices_omp[i].size;
							mp_all_spread_vertices_omp[i].size = 0;
						}
					}
				}
	
				MP_VERTEX_ID_TYPE spread_vertex_id;
				// Take in account only the vertices that have been flagged as
				// spread -> that is, vertices having received a new message.
				#pragma omp for
				for(size_t i = 0; i < mp_all_spread_vertices.size; i++)
				{
					spread_vertex_id = mp_all_spread_vertices.data[i];
					temp_vertex = mp_get_vertex_by_id(spread_vertex_id);
					temp_vertex->has_message = true;
					temp_vertex->message = temp_vertex->message_next;
					temp_vertex->has_message_next = false;
				}
				
				#pragma omp for reduction(+:mp_messages_left)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left += mp_messages_left_omp[i];
					mp_messages_left_omp[i] = 0;
				}
			} // End of OpenMP parallel region
	
			timer_superstep_stop = omp_get_wtime();
			timer_superstep_total += (timer_superstep_stop - timer_superstep_start);
			printf("Meta-superstep %zu superstep %zu finished in %fs; %zu active vertices and %zu messages left.\n", mp_get_meta_superstep(), mp_get_superstep(), timer_superstep_stop - timer_superstep_start, mp_active_vertices, mp_messages_left);
			mp_increment_superstep();
		}
		for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
		{
			mp_get_vertex_by_location(i)->active = true;
		}
		mp_active_vertices = mp_get_vertices_count();
		mp_increment_meta_superstep();	
	}

	printf("Total time of supersteps: %fs.\n", timer_superstep_total);

	// Free and clean program.	
	#pragma omp for
	for(int i = 0; i < OMP_NUM_THREADS; i++)
	{
		mp_safe_free(mp_all_spread_vertices_omp[i].data);
	}
	mp_safe_free(mp_all_spread_vertices.data);

	return 0;
}

void mp_vote_to_halt(struct mp_vertex_t* v)
{
	v->active = false;
}

#endif // COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
