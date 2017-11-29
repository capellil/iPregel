/**
 * @file combiner_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_POSTAMBLE_H_INCLUDED
#define COMBINER_POSTAMBLE_H_INCLUDED

#include <omp.h>

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
	MY_PREGEL_LOCK(&all_vertices[id].lock);
	if(all_vertices[id].has_message_next)
	{
		combine(&all_vertices[id].message_next, &message);
		MY_PREGEL_UNLOCK(&all_vertices[id].lock);
	}
	else
	{
		all_vertices[id].has_message_next = true;
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
	double timer_init_start = omp_get_wtime();
	double timer_init_stop = 0;
	vertices_count = number_of_vertices;
	all_vertices = (struct vertex_t*)safe_malloc(sizeof(struct vertex_t) * (vertices_count + 1));

	unsigned int chunk = vertices_count / 100;
	if(chunk == 0)
	{
		chunk = 1;
	}
	unsigned int progress = 0;
	unsigned int i = 0;
	printf("%3u %% vertices loaded.\r", progress);
	fflush(stdout);
	i++;
	// Deserialise all the vertices
	while(i <= vertices_count && !feof(f))
	{
		all_vertices[i].active = true;
		deserialise_vertex(f, &all_vertices[i]);
		active_vertices++;
		all_vertices[i].has_message = false;
		all_vertices[i].has_message_next = false;
		MY_PREGEL_LOCK_INIT(&all_vertices[i].lock);
		if(i % chunk == 0)
		{
			progress++;
			printf("%3u %%\r", progress);
			fflush(stdout);
		}
		i++;
	}
	printf("100 %%\n");

	timer_init_stop = omp_get_wtime();
	printf("Initialisation finished in %fs.\n", timer_init_stop - timer_init_start);
		
	return 0;
}

int run()
{
	double timer_superstep_total = 0;
	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;
	while(meta_superstep < meta_superstep_count)
	{
		superstep = 0;
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
				for(unsigned int i = 1; i <= vertices_count; i++)
				{
					if(all_vertices[i].active || has_message(&all_vertices[i]))
					{
						all_vertices[i].active = true;
						compute(&all_vertices[i]);
						if(all_vertices[i].active)
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
					}
				}
			}
	
			timer_superstep_stop = omp_get_wtime();
			timer_superstep_total += timer_superstep_stop - timer_superstep_start;
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
	
	printf("Total time of supersteps: %fs.\n", timer_superstep_total);

	return 0;
}

void vote_to_halt(struct vertex_t* v)
{
	v->active = false;
}

#endif // COMBINER_POSTAMBLE_H_INCLUDED
