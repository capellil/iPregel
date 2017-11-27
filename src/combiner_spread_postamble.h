/**
 * @file combiner_spread_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
#define COMBINER_SPREAD_POSTAMBLE_H_INCLUDED

#include <omp.h>
#include <string.h>

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

void add_spread_vertex(VERTEX_ID id)
{
	struct vertex_list_t* my_list = &all_spread_vertices_omp[omp_get_thread_num()];
	if(my_list->size == my_list->max_size)
	{
		my_list->max_size++;
		my_list->data = safe_realloc(my_list->data, sizeof(VERTEX_ID) * my_list->max_size);
	}

	my_list->data[my_list->size] = id;
	my_list->size++;
}

void send_message(VERTEX_ID id, MESSAGE_TYPE message)
{
	MY_PREGEL_LOCK(&all_vertices[id].lock);
	if(all_vertices[id].has_message_next)
	{
		combine(&all_vertices[id].message_next, &message);
		MY_PREGEL_UNLOCK(&all_vertices[id].lock);
	}
	else
	{
		all_vertices[id].has_message_next = true;
		all_vertices[id].active = true;
		all_vertices[id].message_next = message;
		MY_PREGEL_UNLOCK(&all_vertices[id].lock);
		messages_left_omp[omp_get_thread_num()]++;
	}
}

void broadcast(struct vertex_t* v, MESSAGE_TYPE message)
{
	for(unsigned int i = 0; i < v->out_neighbours_count; i++)
	{
		send_message(v->out_neighbours[i], message);
	}
}

int init(FILE* f, unsigned int number_of_vertices)
{
	vertices_count = number_of_vertices;
	all_vertices = (struct vertex_t*)safe_malloc(sizeof(struct vertex_t) * (vertices_count + 1));
	
	all_spread_vertices.max_size = 1;
	all_spread_vertices.size = 0;
	all_spread_vertices.data = safe_malloc(sizeof(VERTEX_ID) * all_spread_vertices.max_size);
	for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
	{
		all_spread_vertices_omp[i].max_size = 1;
		all_spread_vertices_omp[i].size = 0;
		all_spread_vertices_omp[i].data = safe_malloc(sizeof(VERTEX_ID) * all_spread_vertices_omp[i].max_size);
	}

	// Deserialise all the vertices
	for(unsigned int i = 1; i <= vertices_count && !feof(f); i++)
	{
		deserialise_vertex(f, &all_vertices[i]);
		active_vertices++;
	}

	// Allocate the inbox for each vertex in each thread's inbox.
	for(unsigned int i = 1; i <= vertices_count; i++)
	{
		all_vertices[i].active = true;
		all_vertices[i].has_message = false;
		all_vertices[i].has_message_next = false;
		MY_PREGEL_LOCK_INIT(&all_vertices[i].lock);
	}
		
	return 0;
}

int run()
{
	double timer_superstep_total = 0;
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
												  messages_left_omp, \
												  superstep, \
												  spread_vertices_count, \
												  all_spread_vertices, \
												  all_spread_vertices_omp)
		{
			if(superstep == 0)
			{
				#pragma omp for reduction(+:active_vertices)
				for(unsigned int i = 1; i <= vertices_count; i++)
				{
					if(all_vertices[i].active)
					{
						all_vertices[i].active = true;
						compute(&all_vertices[i]);
						if(all_vertices[i].active)
						{
							active_vertices++;
						}
					}
				}
			}
			else
			{
				#pragma omp for reduction(+:active_vertices)
				for(unsigned int i = 0; i < all_spread_vertices.size; i++)
				{
					all_vertices[all_spread_vertices.data[i]].active = true;
					compute(&all_vertices[all_spread_vertices.data[i]]);
					if(all_vertices[all_spread_vertices.data[i]].active)
					{
						active_vertices++;
					}
				}
			}

			#pragma omp for reduction(+:messages_left)
			for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
			{
				messages_left += messages_left_omp[i];
				messages_left_omp[i] = 0;
				all_spread_vertices_omp[i].size = 0;
			}
		
			// Take in account the number of vertices that halted.
			// Swap the message boxes for next superstep.
			#pragma omp for
			for(unsigned int i = 1; i <= vertices_count; i++)
			{
				if(all_vertices[i].has_message_next)
				{
					all_vertices[i].has_message = true;
					all_vertices[i].message = all_vertices[i].message_next;
					all_vertices[i].has_message_next = false;
					add_spread_vertex(i);
				}
			}
			
			#pragma omp for reduction(+:messages_left) reduction(+:spread_vertices_count)
			for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
			{
				messages_left += messages_left_omp[i];
				messages_left_omp[i] = 0;
				spread_vertices_count += all_spread_vertices_omp[i].size;
			}
		}

		if(all_spread_vertices.max_size < spread_vertices_count)
		{
			all_spread_vertices.data = safe_realloc(all_spread_vertices.data, sizeof(VERTEX_ID) *  spread_vertices_count);
			all_spread_vertices.max_size = spread_vertices_count;
		}
		spread_vertices_count = 0;
	
		all_spread_vertices.size = 0;
		for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
		{
			if(all_spread_vertices_omp[i].size > 0)
			{
				memmove(&all_spread_vertices.data[all_spread_vertices.size], all_spread_vertices_omp[i].data, all_spread_vertices_omp[i].size * sizeof(VERTEX_ID));
				all_spread_vertices.size += all_spread_vertices_omp[i].size;
				all_spread_vertices_omp[i].size = 0;
			}
		}

		timer_superstep_stop = omp_get_wtime();
		timer_superstep_total += (timer_superstep_stop - timer_superstep_start);
		printf("Superstep %u finished in %fs; %u active vertices and %u messages left.\n", superstep, timer_superstep_stop - timer_superstep_start, active_vertices, messages_left);
		superstep++;
	}

	printf("Total time of supersteps: %fs.\n", timer_superstep_total);

	return 0;
}

void vote_to_halt(struct vertex_t* v)
{
	v->active = false;
}

#endif // COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
