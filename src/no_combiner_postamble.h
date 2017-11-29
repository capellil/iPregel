/**
 * @file no_combiner_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef NO_COMBINER_POSTAMBLE_H_INCLUDED
#define NO_COMBINER_POSTAMBLE_H_INCLUDED

void append_message_to_mailbox(struct messagebox_t* m, MESSAGE_TYPE message)
{
	if(m->message_number == m->max_message_number)
	{
		m->max_message_number++;
		m->messages = safe_realloc(m->messages, m->max_message_number);
	}
	
	m->messages[m->message_number] = message;
	m->message_number++;
}

bool has_message(struct vertex_t* v)
{
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		if(all_inboxes[i][v->id].message_read < all_inboxes[i][v->id].message_number)
		{
			return true;
		}
	}
	return false;
}

bool get_next_message(struct vertex_t* v, MESSAGE_TYPE* message_value)
{
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		if(all_inboxes[i][v->id].message_read < all_inboxes[i][v->id].message_number)
		{
			*message_value = all_inboxes[i][v->id].messages[all_inboxes[i][v->id].message_read];
			all_inboxes[i][v->id].message_read++;
			messages_left_read_omp[omp_get_thread_num()]++;
			return true;
		}
	}
	return false;
}

void send_message(VERTEX_ID id, MESSAGE_TYPE message)
{
	int me = omp_get_num_threads();
	append_message_to_mailbox(&all_inboxes_next_superstep[me][id], message);
	messages_left_sent_omp[omp_get_thread_num()]++;
}

void broadcast(struct vertex_t* v, MESSAGE_TYPE message)
{
	for(unsigned int i = 0; i < v->out_neighbours_count; i++)
	{
		send_message(v->out_neighbours[i], message);
	}
}

void reset_inbox(VERTEX_ID id)
{
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		#pragma omp atomic
		messages_left -= all_inboxes[i][id].message_number;
		all_inboxes[i][id].message_number = 0;
		all_inboxes[i][id].message_read = 0;
	}
}

int init(FILE* f, unsigned int number_of_vertices)
{
	vertices_count = number_of_vertices;
	all_vertices = (struct vertex_t*)safe_malloc(sizeof(struct vertex_t) * vertices_count);

	#pragma omp parallel for default(none) shared(all_inboxes, \
												  vertices_count, \
												  all_inboxes_next_superstep)
	for(int i = 0; i < omp_get_num_threads(); i++)
	{
		all_inboxes[i] = (struct messagebox_t*)safe_malloc(sizeof(struct messagebox_t) * vertices_count);
		all_inboxes_next_superstep[i] = (struct messagebox_t*)safe_malloc(sizeof(struct messagebox_t) * vertices_count);
	}

	// Deserialise all the vertices
	for(unsigned int i = 0; i < vertices_count && !feof(f); i++)
	{
		all_vertices[i].active = true;
		deserialise_vertex(f, &all_vertices[i]);
		active_vertices++;
	}

	// Allocate the inbox for each vertex in each thread's inbox.
	#pragma omp parallel for default(none) shared(vertices_count, \
												  all_inboxes, \
												  all_inboxes_next_superstep, \
												  all_vertices)
	for(int j = 0; j < omp_get_num_threads(); j++)
	{
		for(unsigned int i = 0; i < vertices_count; i++)
		{
			all_inboxes[j][i].max_message_number = 1;
			all_inboxes[j][i].messages = (MESSAGE_TYPE*)safe_malloc(all_inboxes[j][i].max_message_number);
			all_inboxes[j][i].message_number = 0;
			all_inboxes[j][i].message_read = 0;
			all_inboxes_next_superstep[j][i].max_message_number = 1;
			all_inboxes_next_superstep[j][i].messages = (MESSAGE_TYPE*)safe_malloc(all_inboxes_next_superstep[j][i].max_message_number);
			all_inboxes_next_superstep[j][i].message_number = 0;
			all_inboxes_next_superstep[j][i].message_read = 0;
		}
	}

	return 0;
}

int run()
{
	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;
	while(meta_superstep < meta_superstep_count)
	{
		superstep = 0;
		while(active_vertices != 0 || messages_left > 0)
		{
			timer_superstep_start = omp_get_wtime();
			active_vertices = 0;
			#pragma omp parallel default(none) shared(all_inboxes, \
													  all_inboxes_next_superstep, \
													  vertices_count, \
													  all_vertices, \
													  active_vertices, \
													  messages_left, \
													  messages_left_read_omp, \
													  messages_left_sent_omp)
			{
				#pragma omp for reduction(+:active_vertices)
				for(unsigned int i = 0; i < vertices_count; i++)
				{
					if(all_vertices[i].active || has_message(&all_vertices[i]))
					{
						all_vertices[i].active = true;
						compute(&all_vertices[i]);
						if(all_vertices[i].active)
						{
							active_vertices++;
						}
						reset_inbox(i);
					}
				}
	
				#pragma omp for reduction(+:messages_left)
				for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
				{
					messages_left += messages_left_sent_omp[i];
					messages_left_sent_omp[i] = 0;
				}
				
				#pragma omp for reduction(-:messages_left)
				for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
				{
					messages_left -= messages_left_read_omp[i];
					messages_left_read_omp[i] = 0;
				}
			}
	
			for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
			{
				struct messagebox_t* inbox_swap = all_inboxes[i];
				all_inboxes[i] = all_inboxes_next_superstep[i];
				all_inboxes_next_superstep[i] = inbox_swap;
			}
		
			timer_superstep_stop = omp_get_wtime();
			printf("Meta-superstep %u superstep %u finished in %fs; %u active vertices and %u messages left.\n", meta_superstep, superstep, timer_superstep_stop - timer_superstep_start, active_vertices, messages_left);
			superstep++;
		}
		for(unsigned int i = 0; i < vertices_count; i++)
		{
			all_vertices[i].active = true;
		}
		active_vertices = vertices_count;
		meta_superstep++;
	}
	return 0;
}

void vote_to_halt(struct vertex_t* v)
{
	v->active = false;
}

#endif // NO_COMBINER_POSTAMBLE_H_INCLUDED
