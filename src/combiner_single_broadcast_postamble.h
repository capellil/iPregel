/**
 * @file combiner_single_broadcast_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
#define SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED

#include <omp.h>

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
		struct mp_vertex_t* temp_vertex = NULL;
		while(i < v->in_neighbours_count)
		{
			temp_vertex = mp_get_vertex_by_id(v->in_neighbours[i]);
			if(temp_vertex->has_broadcast_message)
			{
				mp_combine(&v->message, &temp_vertex->broadcast_message);
			}
			i++;
		}
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

	#pragma omp parallel default(none) shared(i) private(temp_vertex)
	for(i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
	{
		temp_vertex = mp_get_vertex_by_location(i);
		temp_vertex->active = true;
		temp_vertex->has_message = false;
		temp_vertex->has_broadcast_message = false;
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
													  mp_messages_left_omp)
			{
				struct mp_vertex_t* temp_vertex = NULL;

				#pragma omp for reduction(+:mp_active_vertices)
				for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
				{
					temp_vertex = mp_get_vertex_by_location(i);	
					temp_vertex->has_broadcast_message = false;
					if(temp_vertex->active || mp_has_message(temp_vertex))
					{
						temp_vertex->active = true;
						mp_compute(temp_vertex);
						if(temp_vertex->active)
						{
							mp_active_vertices++;
						}
					}
				}

				// Count how many messages have been consumed by vertices.	
				#pragma omp for reduction(-:mp_messages_left)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left -= mp_messages_left_omp[i];
					mp_messages_left_omp[i] = 0;
				}

				// Get the messages broadcasted by neighbours.
				#pragma omp for
				for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
				{
					mp_fetch_broadcast_messages(mp_get_vertex_by_location(i));
				}
				
				// Count how many vertices have a message.
				#pragma omp for reduction(+:mp_messages_left)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left += mp_messages_left_omp[i];
					mp_messages_left_omp[i] = 0;
				}
			}
	
			timer_superstep_stop = omp_get_wtime();
			timer_superstep_total += timer_superstep_stop - timer_superstep_start;
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
	
	return 0;
}

void mp_vote_to_halt(struct mp_vertex_t* v)
{
	v->active = false;
}

#endif // SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
