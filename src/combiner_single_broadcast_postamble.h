/**
 * @file combiner_single_broadcast_postamble.h
 * @author Ludovic Capelli
 **/

#ifndef SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
#define SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED

#include <omp.h>

bool ip_has_message(struct ip_vertex_t* v)
{
	return v->has_message;
}

bool ip_get_next_message(struct ip_vertex_t* v, IP_MESSAGE_TYPE* message_value)
{
	if(v->has_message)
	{
		*message_value = v->message;
		v->has_message = false;
		ip_messages_left_omp[omp_get_thread_num()]++;
		return true;
	}
	return false;
}

void ip_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE message)
{
	(void)(id);
	(void)(message);
	printf("The function send_message should not be used in the SINGLE_BROADCAST \
version; only broadcast() should be called, and once per superstep maximum.\n");
	exit(-1);
}

void ip_broadcast(struct ip_vertex_t* v, IP_MESSAGE_TYPE message)
{
	v->has_broadcast_message = true;
	v->broadcast_message = message;
}

void ip_fetch_broadcast_messages(struct ip_vertex_t* v)
{
	IP_NEIGHBOUR_COUNT_TYPE i = 0;
	while(i < v->in_neighbour_count && !ip_get_vertex_by_id(v->in_neighbours[i])->has_broadcast_message)
	{
		i++;
	}

	if(i >= v->in_neighbour_count)
	{
		v->has_message = false;
	}
	else
	{
		ip_messages_left_omp[omp_get_thread_num()]++;
		v->has_message = true;
		v->message = ip_get_vertex_by_id(v->in_neighbours[i])->broadcast_message;
		i++;
		struct ip_vertex_t* temp_vertex = NULL;
		while(i < v->in_neighbour_count)
		{
			temp_vertex = ip_get_vertex_by_id(v->in_neighbours[i]);
			if(temp_vertex->has_broadcast_message)
			{
				ip_combine(&v->message, temp_vertex->broadcast_message);
			}
			i++;
		}
	}	
}

void ip_init_vertex_range(IP_VERTEX_ID_TYPE first, IP_VERTEX_ID_TYPE last)
{
	for(IP_VERTEX_ID_TYPE i = first; i <= last; i++)
	{
		ip_all_vertices[i].id = i - IP_ID_OFFSET;
		ip_all_vertices[i].active = true;
		ip_all_vertices[i].has_message = false;
		ip_all_vertices[i].has_broadcast_message = false;
		#ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
			ip_all_vertices[i].out_neighbour_count = 0;
		#endif // IP_NEEDS_OUT_NEIGHBOUR_COUNT
		#ifdef IP_NEEDS_OUT_NEIGHBOUR_IDS
			ip_all_vertices[i].out_neighbours = NULL;
		#endif // IP_NEEDS_OUT_NEIGHBOUR_IDS
		#ifdef IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
			ip_all_vertices[i].out_neighbour_weights = NULL;
		#endif // IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
		#ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
			ip_all_vertices[i].in_neighbours = NULL;
		#endif
		#ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
			ip_all_vertices[i].in_neighbour_count = 0;
		#endif // IP_NEEDS_IN_NEIGHBOUR_COUNT
		#ifdef IP_NEEDS_IN_NEIGHBOUR_WEIGHTS
			ip_all_vertices[i].in_neighbour_weights = NULL;
		#endif // IP_NEEDS_IN_NEIGHBOUR_WEIGHT
	}
}

void ip_init_specific()
{
	// Initialise OpenMP variables
	#pragma omp parallel
	{
		#pragma omp master
		{
			ip_messages_left_omp = (size_t*)ip_safe_malloc(sizeof(size_t) * ip_thread_count);
			for(int i = 0; i < ip_thread_count; i++)
			{
				ip_messages_left_omp[i] = 0;
			}
		}
	}
}

int ip_run()
{
	double timer_superstep_total = 0;
	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;

	#ifdef IP_ENABLE_THREAD_PROFILING
		double* timer_vertex_compute_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_vertex_compute_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_vertex_compute_total = malloc(sizeof(double) * ip_thread_count);
		double* timer_fetching_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_fetching_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_fetching_total = malloc(sizeof(double) * ip_thread_count);
	#endif

	while(ip_active_vertices != 0 || ip_messages_left > 0)
	{
		timer_superstep_start = omp_get_wtime();
		ip_active_vertices = 0;
		#ifdef IP_ENABLE_THREAD_PROFILING
			#pragma omp parallel default(none) shared(ip_active_vertices, \
													  ip_messages_left, \
													  ip_messages_left_omp, \
													  ip_thread_count, \
													  timer_vertex_compute_start, \
													  timer_vertex_compute_stop, \
													  timer_vertex_compute_total, \
													  timer_fetching_start, \
													  timer_fetching_stop, \
													  timer_fetching_total)
		#else
			#pragma omp parallel default(none) shared(ip_active_vertices, \
													  ip_messages_left, \
													  ip_messages_left_omp, \
													  ip_thread_count)
		#endif
		{
			////////////////////
			// COMPUTE PHASE //
			//////////////////
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_vertex_compute_start[omp_get_thread_num()] = omp_get_wtime();
			#endif
			struct ip_vertex_t* temp_vertex = NULL;
			#pragma omp for reduction(+:ip_active_vertices)
			for(size_t i = 0; i < ip_get_vertices_count(); i++)
			{
				temp_vertex = ip_get_vertex_by_location(i);	
				temp_vertex->has_broadcast_message = false;
				if(temp_vertex->active || ip_has_message(temp_vertex))
				{
					temp_vertex->active = true;
					ip_compute(temp_vertex);
					if(temp_vertex->active)
					{
						ip_active_vertices++;
					}
				}
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_vertex_compute_stop[omp_get_thread_num()] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_vertex_compute_total[omp_get_thread_num()] = timer_vertex_compute_stop[omp_get_thread_num()] - timer_vertex_compute_start[omp_get_thread_num()];
			#endif

			/////////////////////////////
			// MESSAGE COUNTING PHASE //
			///////////////////////////
			// Count how many messages have been consumed by vertices.	
			#pragma omp for reduction(-:ip_messages_left)
			for(int i = 0; i < ip_thread_count; i++)
			{
				ip_messages_left -= ip_messages_left_omp[i];
				ip_messages_left_omp[i] = 0;
			}

			/////////////////////////////
			// MESSAGE FETCHING PHASE //
			///////////////////////////
			// Get the messages broadcasted by neighbours.
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_fetching_start[omp_get_thread_num()] = omp_get_wtime();
			#endif
			#pragma omp for
			for(size_t i = 0; i < ip_get_vertices_count(); i++)
			{
				ip_fetch_broadcast_messages(ip_get_vertex_by_location(i));
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_fetching_stop[omp_get_thread_num()] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_fetching_total[omp_get_thread_num()] = timer_fetching_stop[omp_get_thread_num()] - timer_fetching_start[omp_get_thread_num()];
			#endif
			
			/////////////////////////////
			// MESSAGE COUNTING PHASE //
			///////////////////////////
			// Count how many vertices have a message.
			#pragma omp for reduction(+:ip_messages_left)
			for(int i = 0; i < ip_thread_count; i++)
			{
				ip_messages_left += ip_messages_left_omp[i];
				ip_messages_left_omp[i] = 0;
			}
		}

		timer_superstep_stop = omp_get_wtime();
		timer_superstep_total += timer_superstep_stop - timer_superstep_start;
		printf("Superstep %zu finished in %fs; %zu active vertices and %zu messages left.\n", ip_get_superstep(), timer_superstep_stop - timer_superstep_start, ip_active_vertices, ip_messages_left);
		#ifdef IP_ENABLE_THREAD_PROFILING
			printf("            +");
			for(int i = 0; i < ip_thread_count; i++)
			{
				printf("-----------+");
			}
			printf("\n            |");
			for(int i = 0; i < ip_thread_count; i++)
			{
				printf(" Thread %2d |", i);
			}
			printf("\n+-----------+");
			for(int i = 0; i < ip_thread_count; i++)
			{
				printf("-----------+");
			}
			printf("\n|   Compute |");
			for(int i = 0; i < ip_thread_count; i++)
			{
				printf(" %8.3fs |", timer_vertex_compute_total[i]);
			}
			printf("\n|  Fetching |");
			for(int i = 0; i < ip_thread_count; i++)
			{
				printf(" %8.3fs |", timer_fetching_total[i]);
			}
			printf("\n|     Total |");
			for(int i = 0; i < ip_thread_count; i++)
			{
				printf(" %8.3fs |", timer_vertex_compute_total[i] + timer_fetching_total[i]);
			}
			printf("\n+-----------+");
			for(int i = 0; i < ip_thread_count; i++)
			{
				printf("-----------+");
			}
			printf("\n");
		#endif
		ip_increment_superstep();
	}

	printf("Total time of supersteps: %fs.\n", timer_superstep_total);

	#ifdef IP_ENABLE_THREAD_PROFILING
		free(timer_vertex_compute_start);
		free(timer_vertex_compute_stop);
		free(timer_vertex_compute_total);
		free(timer_fetching_start);
		free(timer_fetching_stop);
		free(timer_fetching_total);
	#endif
	
	return 0;
}

void ip_vote_to_halt(struct ip_vertex_t* v)
{
	v->active = false;
}

#endif // SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
