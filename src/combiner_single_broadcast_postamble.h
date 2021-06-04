/**
 * @file combiner_single_broadcast_postamble.h
 * @copyright Copyright (C) 2019 Ludovic Capelli
 * @par License
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * @author Ludovic Capelli
 **/

#ifndef SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
#define SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED

#include <omp.h>

int ip_my_thread_num;
#pragma omp threadprivate(ip_my_thread_num)

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
	ip_all_neighbour_extras[v->id].has_broadcast_message = true;
	ip_all_neighbour_extras[v->id].broadcast_message = message;
}

void ip_fetch_broadcast_messages(struct ip_vertex_t* v)
{
	IP_NEIGHBOUR_COUNT_TYPE i = 0;
	while(i < v->in_neighbour_count && !ip_all_neighbour_extras[v->in_neighbours[i]].has_broadcast_message)
	{
		i++;
	}

	if(i >= v->in_neighbour_count)
	{
		v->has_message = false;
	}
	else
	{
		if(!v->active)
		{
			#pragma omp atomic
			ip_active_vertices++;
			v->active = true;
		}
		v->has_message = true;
		v->message = ip_all_neighbour_extras[v->in_neighbours[i]].broadcast_message;
		i++;
		while(i < v->in_neighbour_count)
		{
			if(ip_all_neighbour_extras[v->in_neighbours[i]].has_broadcast_message)
			{
				ip_combine(&v->message, ip_all_neighbour_extras[v->in_neighbours[i]].broadcast_message);
			}
			i++;
		}
	}	
}

void ip_init_vertex_range(IP_VERTEX_ID_TYPE first, IP_VERTEX_ID_TYPE last)
{
	for(IP_VERTEX_ID_TYPE i = first; i <= last; i++)
	{
		ip_all_vertices[i].id = i;
		ip_all_vertices[i].active = true;
		ip_all_vertices[i].has_message = false;
		ip_all_neighbour_extras[i].has_broadcast_message = false;
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
	ip_all_neighbour_extras = (struct ip_neighbour_extra_t*)ip_safe_malloc(sizeof(struct ip_neighbour_extra_t) * ip_get_vertices_count());
}

int ip_run()
{
	double timer_superstep_total = 0;
	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;

	#ifdef IP_ENABLE_THREAD_PROFILING
		double* timer_compute_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_compute_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_compute_total = malloc(sizeof(double) * ip_thread_count);
		double* timer_fetching_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_fetching_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_fetching_total = malloc(sizeof(double) * ip_thread_count);
	#endif

	#ifdef IP_ENABLE_THREAD_PROFILING
		#pragma omp parallel default(none) shared(ip_active_vertices, \
												  ip_all_neighbour_extras, \
												  ip_thread_count, \
												  timer_compute_start, \
												  timer_compute_stop, \
												  timer_compute_total, \
												  timer_fetching_start, \
												  timer_fetching_stop, \
												  timer_fetching_total, \
												  timer_superstep_total, \
												  timer_superstep_start, \
												  timer_superstep_stop)
	#else
		#pragma omp parallel default(none) shared(ip_active_vertices, \
												  ip_all_neighbour_extras, \
												  ip_thread_count, \
												  timer_superstep_total, \
												  timer_superstep_start, \
												  timer_superstep_stop)
	#endif
	{
		ip_my_thread_num = omp_get_thread_num();
		while(ip_active_vertices != 0)
		{
			// This barrier is crucial; otherwise a thread may enter the single, change ip_active_vertices before one other thread has entered the loop. Thus the single would never complete.
			#pragma omp barrier

			/////////////////
			// START TIME //
			///////////////
			// This OpenMP single also acts as an implicit barrier to wait for all threads before they start processing a superstep.
			#pragma omp single
			{
				timer_superstep_start = omp_get_wtime();
				ip_active_vertices = 0;
			}

			////////////////////
			// COMPUTE PHASE //
			//////////////////
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_compute_start[ip_my_thread_num] = omp_get_wtime();
			#endif
			struct ip_vertex_t* temp_vertex = NULL;
			#pragma omp for reduction(+:ip_active_vertices) schedule(runtime)
			for(size_t i = 0; i < ip_get_vertices_count(); i++)
			{
				temp_vertex = ip_get_vertex_by_location(i);	
				ip_all_neighbour_extras[temp_vertex->id].has_broadcast_message = false;
				if(temp_vertex->active)
				{
					ip_compute(temp_vertex);
					if(temp_vertex->active)
					{
						ip_active_vertices++;
					}
				}
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_compute_stop[ip_my_thread_num] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_compute_total[ip_my_thread_num] = timer_compute_stop[ip_my_thread_num] - timer_compute_start[ip_my_thread_num];
			#endif

			/////////////////////////////
			// MESSAGE FETCHING PHASE //
			///////////////////////////
			// Get the messages broadcasted by neighbours.
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_fetching_start[ip_my_thread_num] = omp_get_wtime();
			#endif
			#pragma omp for schedule(runtime)
			for(size_t i = 0; i < ip_get_vertices_count(); i++)
			{
				ip_fetch_broadcast_messages(ip_get_vertex_by_location(i));
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_fetching_stop[ip_my_thread_num] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_fetching_total[ip_my_thread_num] = timer_fetching_stop[ip_my_thread_num] - timer_fetching_start[ip_my_thread_num];
			#endif
			
			#pragma omp single
			{
				timer_superstep_stop = omp_get_wtime();
				timer_superstep_total += timer_superstep_stop - timer_superstep_start;
				printf("Superstep%zuDuration:%f\n", ip_get_superstep(), timer_superstep_stop - timer_superstep_start);
				printf("Superstep%zuActiveVertexCount:%zu\n", ip_get_superstep(), ip_active_vertices);
				#ifdef IP_ENABLE_THREAD_PROFILING
					printf("      +------------+----------+-----------+\n");
					printf("      | Processing | Fetching |   Total   |\n");
					printf("+-----+------------+----------+-----------+\n");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf("| %3d |   %8.3f | %8.3f |  %8.3f |\n", i, timer_compute_total[i], timer_fetching_total[i], timer_compute_total[i] + timer_fetching_total[i]);
					}
					printf("+-----+------------+----------+-----------+\n");
				#endif
				ip_increment_superstep();
 			} // End of OpenMP single region
		} // End of superstep processing loop
 	} // End of OpenMP region

	printf("Total time of supersteps: %fs.\n", timer_superstep_total);

	#ifdef IP_ENABLE_THREAD_PROFILING
		free(timer_compute_start);
		free(timer_compute_stop);
		free(timer_compute_total);
		free(timer_fetching_start);
		free(timer_fetching_stop);
		free(timer_fetching_total);
	#endif
	free(ip_all_neighbour_extras);
	
	return 0;
}

void ip_vote_to_halt(struct ip_vertex_t* v)
{
	v->active = false;
}

#endif // SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
