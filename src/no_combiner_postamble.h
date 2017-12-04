/**
 * @file no_combiner_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef NO_COMBINER_POSTAMBLE_H_INCLUDED
#define NO_COMBINER_POSTAMBLE_H_INCLUDED

void mp_append_message_to_mailbox(struct mp_messagebox_t* m, MP_MESSAGE_TYPE message)
{
	if(m->message_number == m->max_message_number)
	{
		m->max_message_number++;
		m->messages = mp_safe_realloc(m->messages, m->max_message_number);
	}
	
	m->messages[m->message_number] = message;
	m->message_number++;
}

bool mp_has_message(struct mp_vertex_t* v)
{
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		if(mp_all_inboxes[i][v->id].message_read < mp_all_inboxes[i][v->id].message_number)
		{
			return true;
		}
	}
	return false;
}

bool mp_get_next_message(struct mp_vertex_t* v, MP_MESSAGE_TYPE* message_value)
{
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		if(mp_all_inboxes[i][v->id].message_read < mp_all_inboxes[i][v->id].message_number)
		{
			*message_value = mp_all_inboxes[i][v->id].messages[mp_all_inboxes[i][v->id].message_read];
			mp_all_inboxes[i][v->id].message_read++;
			mp_messages_left_read_omp[omp_get_thread_num()]++;
			return true;
		}
	}
	return false;
}

void mp_send_message(MP_VERTEX_ID_TYPE id, MP_MESSAGE_TYPE message)
{
	int me = omp_get_num_threads();
	mp_append_message_to_mailbox(&mp_all_inboxes_next_superstep[me][id], message);
	mp_messages_left_sent_omp[omp_get_thread_num()]++;
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

void mp_broadcast(struct mp_vertex_t* v, MP_MESSAGE_TYPE message)
{
	for(MP_NEIGHBOURS_COUNT_TYPE i = 0; i < v->out_neighbours_count; i++)
	{
		mp_send_message(v->out_neighbours[i], message);
	}
}

void mp_reset_inbox(MP_VERTEX_ID_TYPE id)
{
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		#pragma omp atomic
		mp_messages_left -= mp_all_inboxes[i][id].message_number;
		mp_all_inboxes[i][id].message_number = 0;
		mp_all_inboxes[i][id].message_read = 0;
	}
}

int mp_init(FILE* f, size_t number_of_vertices)
{
	mp_set_vertices_count(number_of_vertices);
	mp_all_vertices = (struct mp_vertex_t*)mp_safe_malloc(sizeof(struct mp_vertex_t) * mp_get_vertices_count());

	#pragma omp parallel for default(none) shared(mp_all_inboxes, \
												  mp_all_inboxes_next_superstep)
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		mp_all_inboxes[i] = (struct mp_messagebox_t*)mp_safe_malloc(sizeof(struct mp_messagebox_t) * mp_get_vertices_count());
		mp_all_inboxes_next_superstep[i] = (struct mp_messagebox_t*)mp_safe_malloc(sizeof(struct mp_messagebox_t) * mp_get_vertices_count());
	}

	// Deserialise all the vertices
	for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset() && !feof(f); i++)
	{
		mp_all_vertices[i].active = true;
		mp_deserialise_vertex(f);
		mp_active_vertices++;
	}

	// Allocate the inbox for each vertex in each thread's inbox.
	#pragma omp parallel for default(none) shared(mp_all_inboxes, \
												  mp_all_inboxes_next_superstep, \
												  mp_all_vertices)
	for(int j = 0; j < omp_get_num_threads(); j++)
	{
		for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
		{
			mp_all_inboxes[j][i].max_message_number = 1;
			mp_all_inboxes[j][i].messages = (MP_MESSAGE_TYPE*)mp_safe_malloc(mp_all_inboxes[j][i].max_message_number);
			mp_all_inboxes[j][i].message_number = 0;
			mp_all_inboxes[j][i].message_read = 0;
			mp_all_inboxes_next_superstep[j][i].max_message_number = 1;
			mp_all_inboxes_next_superstep[j][i].messages = (MP_MESSAGE_TYPE*)mp_safe_malloc(mp_all_inboxes_next_superstep[j][i].max_message_number);
			mp_all_inboxes_next_superstep[j][i].message_number = 0;
			mp_all_inboxes_next_superstep[j][i].message_read = 0;
		}
	}

	return 0;
}

int mp_run()
{
	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;
	while(mp_get_meta_superstep() < mp_get_meta_superstep_count())
	{
		mp_reset_superstep();
		while(mp_active_vertices != 0 || mp_messages_left > 0)
		{
			timer_superstep_start = omp_get_wtime();
			mp_active_vertices = 0;
			#pragma omp parallel default(none) shared(mp_all_inboxes, \
													  mp_all_inboxes_next_superstep, \
													  mp_active_vertices, \
													  mp_messages_left, \
													  mp_messages_left_read_omp, \
													  mp_messages_left_sent_omp)
			{
				struct mp_vertex_t* temp_vertex = NULL;

				#pragma omp for reduction(+:mp_active_vertices)
				for(size_t i = mp_get_id_offset(); i < mp_get_vertices_count() + mp_get_id_offset(); i++)
				{
					temp_vertex = mp_get_vertex_by_location(i);
					if(temp_vertex->active || mp_has_message(temp_vertex))
					{
						temp_vertex->active = true;
						mp_compute(temp_vertex);
						if(temp_vertex->active)
						{
							mp_active_vertices++;
						}
						mp_reset_inbox(i);
					}
				}
	
				#pragma omp for reduction(+:mp_messages_left)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left += mp_messages_left_sent_omp[i];
					mp_messages_left_sent_omp[i] = 0;
				}
				
				#pragma omp for reduction(-:mp_messages_left)
				for(int i = 0; i < OMP_NUM_THREADS; i++)
				{
					mp_messages_left -= mp_messages_left_read_omp[i];
					mp_messages_left_read_omp[i] = 0;
				}
			}
	
			for(int i = 0; i < OMP_NUM_THREADS; i++)
			{
				struct mp_messagebox_t* inbox_swap = mp_all_inboxes[i];
				mp_all_inboxes[i] = mp_all_inboxes_next_superstep[i];
				mp_all_inboxes_next_superstep[i] = inbox_swap;
			}
		
			timer_superstep_stop = omp_get_wtime();
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
	return 0;
}

void mp_vote_to_halt(struct mp_vertex_t* v)
{
	v->active = false;
}

#endif // NO_COMBINER_POSTAMBLE_H_INCLUDED
