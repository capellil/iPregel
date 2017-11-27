/**
 * @file single_broadcast_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
#define SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED

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
		messages_left_omp[omp_get_thread_num()]++;
		return true;
	}
	return false;
}

void send_message(VERTEX_ID id, MESSAGE_TYPE message)
{
	(void)(id);
	(void)(message);
	printf("The function send_message should not be used in the SINGLE_BROADCAST \
version; only broadcast() should be called, and once per superstep maximum.\n");
	exit(-1);
}

void broadcast(struct vertex_t* v, MESSAGE_TYPE message)
{
	v->has_broadcast_message = true;
	v->broadcast_message = message;
}

void fetch_broadcast_messages(struct vertex_t* v)
{
	unsigned int i = 0;
	while(i < v->in_neighbours_count && !all_vertices[v->in_neighbours[i]].has_broadcast_message)
	{
		i++;
	}

	if(i >= v->in_neighbours_count)
	{
		v->has_message = false;
	}
	else
	{
		messages_left_omp[omp_get_thread_num()]++;
		v->has_message = true;
		v->message = all_vertices[v->in_neighbours[i]].broadcast_message;
		i++;
		while(i < v->in_neighbours_count)
		{
			if(all_vertices[v->in_neighbours[i]].has_broadcast_message)
			{
				combine(&v->message, &all_vertices[v->in_neighbours[i]].broadcast_message);
			}
			i++;
		}
	}	
}

int init(FILE* f, unsigned int number_of_vertices)
{
	double timer_init_start = omp_get_wtime();
	double timer_init_stop = 0;

	vertices_count = number_of_vertices;
	all_vertices = (struct vertex_t*)safe_malloc(sizeof(struct vertex_t) * (vertices_count + 1));

	unsigned int chunk = vertices_count / 100;
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
		all_vertices[i].has_broadcast_message = false;
		if(i % chunk == 0)
		{
			progress++;
			printf("%3u %%\r", progress);
			fflush(stdout);
		}
		i++;
	}
	printf("\n");

	timer_init_stop = omp_get_wtime();
	printf("Initialisation finished in %fs.\n", timer_init_stop - timer_init_start);
	
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
												  messages_left_omp)
		{
			#pragma omp for reduction(+:active_vertices)
			for(unsigned int i = 1; i <= vertices_count; i++)
			{
				if(all_vertices[i].active || has_message(&all_vertices[i]))
				{
					all_vertices[i].active = true;
					all_vertices[i].has_broadcast_message = false;
					compute(&all_vertices[i]);
					if(all_vertices[i].active)
					{
						active_vertices++;
					}
				}
			}
		
			// Count how many messages have been consumed by vertices.	
			#pragma omp for reduction(-:messages_left)
			for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
			{
				messages_left -= messages_left_omp[i];
				messages_left_omp[i] = 0;
			}

			// Get the messages broadcasted by neighbours.
			#pragma omp for
			for(unsigned int i = 1; i <= vertices_count; i++)
			{
				fetch_broadcast_messages(&all_vertices[i]);
			}

			// Count how many vertices have a message.
			#pragma omp for reduction(+:messages_left)
			for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
			{
				messages_left += messages_left_omp[i];
				messages_left_omp[i] = 0;
			}
		}

		timer_superstep_stop = omp_get_wtime();
		timer_superstep_total += timer_superstep_stop - timer_superstep_start;
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

#endif // SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
