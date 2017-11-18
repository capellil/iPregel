#ifndef X_POSTAMBLE_H_INCLUDED
#define X_POSTAMBLE_H_INCLUDED

#ifdef USE_COMBINER
	bool has_message(struct vertex_t* v)
	{
		return v->has_message;
	}

	bool get_next_message(struct vertex_t* v, MESSAGE_TYPE* message_value)
	{
		if(v->has_message)
		{
			*message_value = v->message;
			v->has_message = false;
			messages_left_omp[omp_get_thread_num()]--;
			return true;
		}
		return false;
	}

	void send_message(VERTEX_ID id, MESSAGE_TYPE message)
	{
		pthread_mutex_lock(&all_vertices[id].mutex);
	
		if(all_vertices[id].has_message_next)
		{
			combine(&all_vertices[id].message_next, &message);
			pthread_mutex_unlock(&all_vertices[id].mutex);
		}
		else
		{
			all_vertices[id].has_message_next = true;
			all_vertices[id].message_next = message;
			pthread_mutex_unlock(&all_vertices[id].mutex);
			messages_left_omp[omp_get_thread_num()]++;
		}
	}

	int init(FILE* f, unsigned int number_of_vertices)
	{
		vertices_count = number_of_vertices;
		all_vertices = (struct vertex_t*)safe_malloc(sizeof(struct vertex_t) * vertices_count);

		// Deserialise all the vertices
		for(unsigned int i = 0; i < vertices_count && !feof(f); i++)
		{
			deserialiseVertex(f, &all_vertices[i]);
			active_vertices++;
		}

		// Allocate the inbox for each vertex in each thread's inbox.
		for(unsigned int i = 0; i < vertices_count; i++)
		{
			all_vertices[i].voted_to_halt = false;
			all_vertices[i].has_message = false;
			all_vertices[i].has_message_next = false;
			pthread_mutex_init(&all_vertices[i].mutex, NULL);
		}
		
		return 0;
	}

	int run()
	{
		double timer_superstep_start = 0;
		double timer_superstep_stop = 0;
		while(active_vertices != 0 || messages_left > 0)
		{
			timer_superstep_start = omp_get_wtime();
			active_vertices = 0;
			#pragma omp parallel default(none) shared(vertices_count, \
													  all_vertices, \
													  active_vertices, \
													  messages_left, \
													  messages_left_omp)
			{
				#pragma omp for reduction(+:active_vertices)
				for(unsigned int i = 0; i < vertices_count; i++)
				{
					if(all_vertices[i].active || has_message(&all_vertices[i]))
					{
						all_vertices[i].active = true;
						active_vertices++;
						compute(&all_vertices[i]);
					}
				}

				#pragma omp for reduction(+:messages_left)
				for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
				{
					messages_left += messages_left_omp[i];
					messages_left_omp[i] = 0;
				}
			
				// Take in account the number of vertices that halted.
				// Swap the message boxes for next superstep.
				#pragma omp for reduction(-:active_vertices)
				for(unsigned int i = 0; i < vertices_count; i++)
				{
					if(all_vertices[i].voted_to_halt)
					{
						active_vertices--;
						all_vertices[i].voted_to_halt = false;
					}
	
					if(all_vertices[i].has_message_next)
					{
						all_vertices[i].has_message = true;
						all_vertices[i].message = all_vertices[i].message_next;
						all_vertices[i].has_message_next = false;
					}
				}
			}

			timer_superstep_stop = omp_get_wtime();
			printf("Superstep %u finished in %fs; %u active vertices and %u messages left.\n", superstep, timer_superstep_stop - timer_superstep_start, active_vertices, messages_left);
			superstep++;
		}

		return 0;
	}
#else
	bool has_message(struct vertex_t* v)
	{
		for(unsigned int i = 0; i < omp_get_num_threads(); i++)
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
		for(unsigned int i = 0; i < omp_get_num_threads(); i++)
		{
			if(all_inboxes[i][v->id].message_read < all_inboxes[i][v->id].message_number)
			{
				*message_value = all_inboxes[i][v->id].messages[all_inboxes[i][v->id].message_read];
				all_inboxes[i][v->id].message_read++;
				messages_left_omp[omp_get_thread_num()]--;
				return true;
			}
		}
		return false;
	}

	void send_message(VERTEX_ID id, MESSAGE_TYPE message)
	{
		all_inboxes_next_superstep[omp_get_thread_num()][id].message_number++;
		if(all_inboxes_next_superstep[omp_get_thread_num()][id].message_number > all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number)
		{
			all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number = all_inboxes_next_superstep[omp_get_thread_num()][id].message_number;
			all_inboxes_next_superstep[omp_get_thread_num()][id].messages = safe_realloc(all_inboxes_next_superstep[omp_get_thread_num()][id].messages, sizeof(MESSAGE_TYPE) * all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number);
		}
		
		all_inboxes_next_superstep[omp_get_thread_num()][id].messages[all_inboxes_next_superstep[omp_get_thread_num()][id].message_number-1] = message;
		messages_left_omp[omp_get_thread_num()]++;
	}

	void reset_inbox(VERTEX_ID id)
	{
		for(unsigned int i = 0; i < omp_get_num_threads(); i++)
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
		#pragma omp parallel for default(none) shared(all_inboxes, \
													  vertices_count, \
													  all_inboxes_next_superstep)
		for(unsigned int i = 0; i < omp_get_num_threads(); i++)
		{
			all_inboxes[i] = (struct messagebox_t*)safe_malloc(sizeof(struct messagebox_t) * vertices_count);
			all_inboxes_next_superstep[i] = (struct messagebox_t*)safe_malloc(sizeof(struct messagebox_t) * vertices_count);
		}
	
		// Deserialise all the vertices
		for(unsigned int i = 0; i < vertices_count && !feof(f); i++)
		{
			deserialiseVertex(f, &all_vertices[i]);
			active_vertices++;
		}

		// Allocate the inbox for each vertex in each thread's inbox.
		#pragma omp parallel for default(none) shared(vertices_count, \
													  all_inboxes, \
													  all_inboxes_next_superstep, \
													  all_vertices)
		for(unsigned int j = 0; j < omp_get_num_threads(); j++)
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
													  messages_left_omp)
			{
				#pragma omp for reduction(+:active_vertices)
				for(unsigned int i = 0; i < vertices_count; i++)
				{
					if(all_vertices[i].active || has_message(&all_vertices[i]))
					{
						all_vertices[i].active = true;
						active_vertices++;
						compute(&all_vertices[i]);
						reset_inbox(i);
					}
				}

				#pragma omp for reduction(+:messages_left)
				for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
				{
					messages_left += messages_left_omp[i];
					messages_left_omp[i] = 0;
				}
			
				#pragma omp for reduction(-:active_vertices)
				for(unsigned int i = 0; i < vertices_count; i++)
				{
					if(all_vertices[i].voted_to_halt)
					{
						active_vertices--;
						all_vertices[i].voted_to_halt = false;
					}
				}
			}

			for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
			{
				struct messagebox_t* inbox_swap = all_inboxes[i];
				all_inboxes[i] = all_inboxes_next_superstep[i];
				all_inboxes_next_superstep[i] = inbox_swap;
			}
		
			timer_superstep_stop = omp_get_wtime();
			printf("Superstep %u finished in %fs; %u active vertices and %u messages left.\n", superstep, timer_superstep_stop - timer_superstep_start, active_vertices, messages_left);
			superstep++;
		}
	}
#endif // if(n)def USE_COMBINER

void vote_to_halt(struct vertex_t* v)
{
	v->active = false;
	v->voted_to_halt = true;
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

#endif // X_POSTAMBLE_H_INCLUDED
