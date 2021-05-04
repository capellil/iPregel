/**
 * @file combiner_spread_single_broadcast_postamble.h
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

#ifndef COMBINER_SPREAD_SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
#define COMBINER_SPREAD_SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED

#include <omp.h>
#include <string.h>

int ip_my_thread_num;
#pragma omp threadprivate(ip_my_thread_num)

void ip_add_target(IP_VERTEX_ID_TYPE id)
{
	if(ip_all_targets.size == ip_all_targets.max_size)
	{
		ip_all_targets.max_size++;
		ip_all_targets.data = ip_safe_realloc(ip_all_targets.data, sizeof(IP_VERTEX_ID_TYPE) * ip_all_targets.max_size);
	}

	ip_all_targets.data[ip_all_targets.size] = id;
	ip_all_targets.size++;
}

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
	ip_all_externalised_structures_1[v->id].has_broadcast_message = true;
	ip_all_externalised_structures_1[v->id].broadcast_message = message;
	for(IP_NEIGHBOUR_COUNT_TYPE i = 0; i < v->out_neighbour_count; i++)
	{
		/* Should use "#pragma omp atomic write" to protect the data race, but
		 * since all threads would race to put the same value in the variable,
		 * it has been purposely left unprotected.
		 */
		ip_all_externalised_structures_2[v->out_neighbours[i]].broadcast_target = true;
	}
}

void ip_fetch_broadcast_messages(struct ip_vertex_t* v)
{
	IP_NEIGHBOUR_COUNT_TYPE i = 0;
	while(i < v->in_neighbour_count && !ip_all_externalised_structures_1[v->in_neighbours[i]].has_broadcast_message)
	{
		i++;
	}

	if(i >= v->in_neighbour_count)
	{
		v->has_message = false;
	}
	else
	{
		v->has_message = true;
		v->message = ip_all_externalised_structures_1[v->in_neighbours[i]].broadcast_message;
		i++;
		while(i < v->in_neighbour_count)
		{
			if(ip_all_externalised_structures_1[v->in_neighbours[i]].has_broadcast_message)
			{
				ip_combine(&v->message, ip_all_externalised_structures_1[v->in_neighbours[i]].broadcast_message);
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
		ip_all_externalised_structures_2[i].broadcast_target = false;
		ip_all_vertices[i].has_message = false;
		ip_all_externalised_structures_1[i].has_broadcast_message = false;
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
		ip_all_targets.data[i] = ip_all_vertices[i].id;
	}
}

void ip_init_specific()
{
	ip_all_targets.max_size = ip_get_vertices_count();
	ip_all_targets.size = ip_get_vertices_count();
	ip_all_targets.data = ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * ip_all_targets.max_size);
	ip_all_externalised_structures_1 = (struct ip_externalised_structure_1_t*)ip_safe_malloc(sizeof(struct ip_externalised_structure_1_t) * ip_get_vertices_count());
	ip_all_externalised_structures_2 = (struct ip_externalised_structure_2_t*)ip_safe_malloc(sizeof(struct ip_externalised_structure_2_t) * ip_get_vertices_count());
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
		double* timer_target_filtering_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_target_filtering_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_target_filtering_total = malloc(sizeof(double) * ip_thread_count);
		double* timer_message_fetching_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_message_fetching_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_message_fetching_total = malloc(sizeof(double) * ip_thread_count);
		double* timer_state_reseting_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_state_reseting_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_state_reseting_total = malloc(sizeof(double) * ip_thread_count);
		size_t* timer_edge_count = malloc(sizeof(size_t) * ip_thread_count);
		size_t timer_edge_count_total = 0;
	#endif

	#ifdef IP_ENABLE_THREAD_PROFILING
		#pragma omp parallel default(none) shared(ip_all_targets, \
												  ip_thread_count, \
												  ip_all_externalised_structures_1, \
												  ip_all_externalised_structures_2, \
												  ip_active_vertices, \
												  timer_compute_start, \
												  timer_compute_stop, \
												  timer_compute_total, \
												  timer_target_filtering_start, \
												  timer_target_filtering_stop, \
												  timer_target_filtering_total, \
												  timer_message_fetching_start, \
												  timer_message_fetching_stop, \
												  timer_message_fetching_total, \
												  timer_state_reseting_start, \
												  timer_state_reseting_stop, \
												  timer_state_reseting_total, \
												  timer_edge_count, \
												  timer_edge_count_total, \
												  timer_superstep_total, \
												  timer_superstep_start, \
												  timer_superstep_stop)
	#else
		#pragma omp parallel default(none) shared(ip_all_targets, \
												  ip_thread_count, \
												  ip_all_externalised_structures_1, \
												  ip_all_externalised_structures_2, \
												  ip_active_vertices, \
												  timer_superstep_total, \
												  timer_superstep_start, \
												  timer_superstep_stop)
	#endif
	{	
		ip_my_thread_num = omp_get_thread_num();
		while(ip_is_first_superstep() || ip_all_targets.size > 0)
		{
			/////////////////
			// START TIME //
			///////////////
			// This OpenMP single also acts as an implicit barrier to wait for all threads before they start processing a superstep.
			#pragma omp single
			{
				timer_superstep_start = omp_get_wtime();
			}

			////////////////////
			// COMPUTE PHASE //
			//////////////////
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_compute_start[ip_my_thread_num] = omp_get_wtime();
				timer_compute_stop[ip_my_thread_num] = timer_compute_start[ip_my_thread_num];
				timer_edge_count[ip_my_thread_num] = 0;
			#endif
			struct ip_vertex_t* temp_vertex = NULL;
			#ifdef IP_ENABLE_THREAD_PROFILING
				#pragma omp for reduction(+:timer_edge_count_total) schedule(runtime)
			#else
				#pragma omp for schedule(runtime)
			#endif
			for(size_t i = 0; i < ip_all_targets.size; i++)
			{
				temp_vertex = ip_get_vertex_by_id(ip_all_targets.data[i]);
				ip_compute(temp_vertex);
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_compute_stop[ip_my_thread_num] = omp_get_wtime();
					timer_edge_count[ip_my_thread_num] += temp_vertex->in_neighbour_count;
					timer_edge_count_total += temp_vertex->out_neighbour_count;
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_compute_total[ip_my_thread_num] = timer_compute_stop[ip_my_thread_num] - timer_compute_start[ip_my_thread_num];
			#endif
		
			/////////////////////////////
			// TARGET FILTERING PHASE //
			///////////////////////////
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_target_filtering_start[ip_my_thread_num] = omp_get_wtime();
				timer_target_filtering_stop[ip_my_thread_num] = timer_target_filtering_start[ip_my_thread_num];
			#endif
			#pragma omp single
			{
				ip_all_targets.size = 0;
				for(size_t i = 0; i < ip_get_vertices_count(); i++)
				{
					temp_vertex = ip_get_vertex_by_location(i);
					if(ip_all_externalised_structures_2[i].broadcast_target)
					{
						ip_add_target(temp_vertex->id);
					}
				}
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_target_filtering_stop[ip_my_thread_num] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_target_filtering_total[ip_my_thread_num] = timer_target_filtering_stop[ip_my_thread_num] - timer_target_filtering_start[ip_my_thread_num];
			#endif
	
			/////////////////////////////
			// MESSAGE FETCHING PHASE //
			///////////////////////////
			// Get the messages broadcasted by neighbours, but only for those
			// who have neighbours who broadcasted.
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_message_fetching_start[ip_my_thread_num] = omp_get_wtime();
				timer_message_fetching_stop[ip_my_thread_num] = timer_message_fetching_start[ip_my_thread_num];
			#endif
			#pragma omp for schedule(runtime)
			for(size_t i = 0; i < ip_all_targets.size; i++)
			{
				temp_vertex = ip_get_vertex_by_id(ip_all_targets.data[i]);
				if(ip_all_externalised_structures_2[temp_vertex->id].broadcast_target)
				{
					ip_fetch_broadcast_messages(temp_vertex);
					ip_all_externalised_structures_2[temp_vertex->id].broadcast_target = false;
				}
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_message_fetching_stop[ip_my_thread_num] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_message_fetching_total[ip_my_thread_num] = timer_message_fetching_stop[ip_my_thread_num] - timer_message_fetching_start[ip_my_thread_num];
			#endif

			///////////////////////////
			// STATE RESETING PHASE //
			/////////////////////////
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_state_reseting_start[ip_my_thread_num] = omp_get_wtime();
				timer_state_reseting_stop[ip_my_thread_num] = timer_state_reseting_start[ip_my_thread_num];
			#endif
			#pragma omp for schedule(runtime)
			for(size_t i = 0; i < ip_get_vertices_count(); i++)
			{
				ip_all_externalised_structures_1[i].has_broadcast_message = false;
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_state_reseting_stop[ip_my_thread_num] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_state_reseting_total[ip_my_thread_num] = timer_state_reseting_stop[ip_my_thread_num] - timer_state_reseting_start[ip_my_thread_num];
			#endif
			
			#pragma omp single
			{
				timer_superstep_stop = omp_get_wtime();
				timer_superstep_total += timer_superstep_stop - timer_superstep_start;
				printf("Superstep%zuDuration:%f\n", ip_get_superstep(), timer_superstep_stop - timer_superstep_start);
				printf("Superstep%zuActiveVertexCount:%zu\n", ip_get_superstep(), ip_active_vertices);
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
						printf(" %8.3fs |", timer_compute_total[i]);
					}
					printf("\n| Filtering |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3fs |", timer_target_filtering_total[i]);
					}
					printf("\n|  Fetching |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3fs |", timer_message_fetching_total[i]);
					}
					printf("\n|     Reset |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3fs |", timer_state_reseting_total[i]);
					}
					printf("\n|     Total |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3fs |", timer_message_fetching_total[i] + timer_target_filtering_total[i] + timer_compute_total[i] + timer_state_reseting_total[i]);
					}
					printf("\n| EdgeCount |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3f%% |", 100.0 * (((double)timer_edge_count[i]) / ((double)timer_edge_count_total)));
					}
					printf("\n+-----------+");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf("-----------+");
					}
					printf("\n");
					timer_edge_count_total = 0;
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
		free(timer_target_filtering_start);
		free(timer_target_filtering_stop);
		free(timer_target_filtering_total);
		free(timer_message_fetching_start);
		free(timer_message_fetching_stop);
		free(timer_message_fetching_total);
		free(timer_state_reseting_start);
		free(timer_state_reseting_stop);
		free(timer_state_reseting_total);
	#endif
	free(ip_all_externalised_structures_1);
	free(ip_all_externalised_structures_2);
	
	return 0;
}

void ip_vote_to_halt(struct ip_vertex_t* v)
{
	(void)(v);
}

#endif // COMBINER_SPREAD_SINGLE_BROADCAST_POSTAMBLE_H_INCLUDED
