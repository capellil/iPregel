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
		MY_PREGEL_LOCK_INIT(&all_vertices[i].lock);
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

void vote_to_halt(struct vertex_t* v)
{
	v->active = false;
	v->voted_to_halt = false;
}

#endif // COMBINER_POSTAMBLE_H_INCLUDED
