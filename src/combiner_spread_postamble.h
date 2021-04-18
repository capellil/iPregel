/**
 * @file combiner_spread_postamble.h
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

#ifndef COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
#define COMBINER_SPREAD_POSTAMBLE_H_INCLUDED

#include <omp.h>
#include <string.h>

#define IP_CACHE_LINE_LENGTH sizeof(void*)

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

void ip_add_spread_vertex(IP_VERTEX_ID_TYPE id)
{
	struct ip_vertex_list_t* my_list = &ip_all_spread_vertices_omp[ip_my_thread_num * IP_CACHE_LINE_LENGTH];
	if(my_list->size == my_list->max_size)
	{
		my_list->max_size++;
		my_list->data = ip_safe_realloc(my_list->data, sizeof(IP_VERTEX_ID_TYPE) * my_list->max_size);
	}

	my_list->data[my_list->size] = id;
	my_list->size++;
}

void ip_cas(IP_VERTEX_ID_TYPE id, IP_VERTEX_ID_TYPE message)
{
	IP_MESSAGE_TYPE old_value = ip_all_externalised_structures[id].message_next;
	IP_MESSAGE_TYPE new_value = old_value;
	ip_combine(&new_value, message);
	while(new_value != old_value && !atomic_compare_exchange_strong(&ip_all_externalised_structures[id].message_next, &old_value, new_value))
	{
		old_value = ip_all_externalised_structures[id].message_next;
		new_value = old_value;
		ip_combine(&new_value, message);
	}
}


void ip_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE message)
{
	if(ip_all_externalised_structures[id].has_message_next)
	{
		ip_cas(id, message);
	}
	else
	{
		ip_lock_acquire(&ip_all_externalised_structures[id].lock);
		if(ip_all_externalised_structures[id].has_message_next)
		{
			// During the time we were waiting to acquire the lock, someone else was having the lock and wrote the first value in the temp_vertex mailbox.
			// We can release the lock and do the CAS combination straight away
			ip_lock_release(&ip_all_externalised_structures[id].lock);
			ip_cas(id, message);
		}
		else
		{
			// We are still the first one waiting to write in that vertex mailbox
			ip_all_externalised_structures[id].message_next = message;
			ip_all_externalised_structures[id].has_message_next = true;
			ip_lock_release(&ip_all_externalised_structures[id].lock);
			ip_add_spread_vertex(id);
		}
	}
}

void ip_broadcast(struct ip_vertex_t* v, IP_MESSAGE_TYPE message)
{
	for(IP_NEIGHBOUR_COUNT_TYPE i = 0; i < v->out_neighbour_count; i++)
	{
		ip_send_message(v->out_neighbours[i], message);
	}
}

void ip_init_vertex_range(IP_VERTEX_ID_TYPE first, IP_VERTEX_ID_TYPE last)
{
	for(IP_VERTEX_ID_TYPE i = first; i <= last; i++)
	{
		ip_all_vertices[i].id = i;
		ip_all_vertices[i].has_message = false;
		ip_all_externalised_structures[i].has_message_next = false;
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
		ip_lock_init(&ip_all_externalised_structures[i].lock);
	}
}

void ip_init_specific()
{
	// Initialise OpenMP variables
	#pragma omp parallel
	{
		#pragma omp master
		{
			ip_all_spread_vertices_omp = (struct ip_vertex_list_t*)ip_safe_malloc(sizeof(struct ip_vertex_list_t) * ip_thread_count * IP_CACHE_LINE_LENGTH);
		}
	}

	ip_all_spread_vertices.max_size = 1;
	ip_all_spread_vertices.size = 0;
	ip_all_spread_vertices.data = ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * ip_all_spread_vertices.max_size);
	#pragma omp parallel default(none) shared(ip_all_spread_vertices_omp)
	{
		ip_all_spread_vertices_omp[omp_get_thread_num() * IP_CACHE_LINE_LENGTH].max_size = 1;
		ip_all_spread_vertices_omp[omp_get_thread_num() * IP_CACHE_LINE_LENGTH].size = 0;
		ip_all_spread_vertices_omp[omp_get_thread_num() * IP_CACHE_LINE_LENGTH].data = ip_safe_malloc(sizeof(IP_VERTEX_ID_TYPE) * ip_all_spread_vertices_omp[omp_get_thread_num() * IP_CACHE_LINE_LENGTH].max_size);
	}
	ip_all_externalised_structures = (struct ip_externalised_structure_t*)ip_safe_malloc(sizeof(struct ip_externalised_structure_t) * ip_get_vertices_count());
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
		double* timer_spread_merge_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_spread_merge_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_spread_merge_total = malloc(sizeof(double) * ip_thread_count);
		double* timer_mailbox_update_start = malloc(sizeof(double) * ip_thread_count);
		double* timer_mailbox_update_stop = malloc(sizeof(double) * ip_thread_count);
		double* timer_mailbox_update_total = malloc(sizeof(double) * ip_thread_count);
		size_t* timer_edge_count = malloc(sizeof(size_t) * ip_thread_count);
		size_t timer_edge_count_total = 0;
	#endif

	#ifdef IP_ENABLE_THREAD_PROFILING
		#pragma omp parallel default(none) shared(ip_active_vertices, \
												  ip_all_spread_vertices, \
												  ip_all_spread_vertices_omp, \
												  ip_thread_count, \
												  ip_all_externalised_structures, \
												  timer_compute_start, \
												  timer_compute_stop, \
												  timer_compute_total, \
												  timer_spread_merge_start, \
												  timer_spread_merge_stop, \
												  timer_spread_merge_total, \
												  timer_mailbox_update_start, \
												  timer_mailbox_update_stop, \
												  timer_mailbox_update_total, \
												  timer_edge_count, \
												  timer_edge_count_total, \
												  timer_superstep_total, \
												  timer_superstep_start, \
												  timer_superstep_stop)
	#else
		#pragma omp parallel default(none) shared(ip_active_vertices, \
												  ip_all_spread_vertices, \
												  ip_all_spread_vertices_omp, \
												  ip_thread_count, \
												  ip_all_externalised_structures, \
												  timer_superstep_total, \
												  timer_superstep_start, \
												  timer_superstep_stop)
	#endif
	{
		ip_my_thread_num = omp_get_thread_num();
		while(ip_is_first_superstep() || ip_active_vertices > 0)
		{
			// This barrier is crucial; otherwise a thread may enter the single, change ip_active_vertices before one other thread has entered the loop. Thus the single would never complete.
			#pragma omp barrier

			//////////////////
			// START TIMER //
			////////////////
			// This OpenMP single also acts as an implicit barrier to wait for all threads before they start processing a superstep.
			#pragma omp single
			{
				ip_active_vertices = 0;
				timer_superstep_start = omp_get_wtime();
			}
			
			////////////////////
			// COMPUTE PHASE //
			//////////////////
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_compute_start[ip_my_thread_num] = omp_get_wtime();
				timer_edge_count[ip_my_thread_num] = 0;
			#endif
			struct ip_vertex_t* temp_vertex = NULL;
			if(ip_is_first_superstep())
			{
				#ifdef IP_ENABLE_THREAD_PROFILING
					#pragma omp for reduction(+:timer_edge_count_total) schedule(runtime)
				#else
					#pragma omp for schedule(runtime)
				#endif
				for(size_t i = 0; i < ip_get_vertices_count(); i++)
				{
					temp_vertex = ip_get_vertex_by_location(i);
					ip_compute(temp_vertex);
					#ifdef IP_ENABLE_THREAD_PROFILING
						timer_compute_stop[ip_my_thread_num] = omp_get_wtime();
						timer_edge_count[ip_my_thread_num] += temp_vertex->out_neighbour_count;
						timer_edge_count_total += temp_vertex->out_neighbour_count;
					#endif
				}
			}
			else
			{
				IP_VERTEX_ID_TYPE spread_neighbour_id;
				#ifdef IP_ENABLE_THREAD_PROFILING
					#pragma omp for reduction(+:timer_edge_count_total) schedule(runtime)
				#else
					#pragma omp for schedule(runtime)
				#endif
				for(size_t i = 0; i < ip_all_spread_vertices.size; i++)
				{
					spread_neighbour_id = ip_all_spread_vertices.data[i];
					temp_vertex = ip_get_vertex_by_id(spread_neighbour_id);
					ip_compute(temp_vertex);
					#ifdef IP_ENABLE_THREAD_PROFILING
						timer_compute_stop[ip_my_thread_num] = omp_get_wtime();
						timer_edge_count[ip_my_thread_num] += temp_vertex->out_neighbour_count;
						timer_edge_count_total += temp_vertex->out_neighbour_count;
					#endif
				}
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_compute_total[ip_my_thread_num] = timer_compute_stop[ip_my_thread_num] - timer_compute_start[ip_my_thread_num];
			#endif
			
			////////////////////////////
			// ACTIVE VERTICES COUNT //
			//////////////////////////

			#pragma omp atomic
			ip_active_vertices += ip_all_spread_vertices_omp[ip_my_thread_num * IP_CACHE_LINE_LENGTH].size;

			// This barrier is crucial; it makes sure that no thread can enter the single below, which uses ip_active_vertices, before every thread incremented it ip_active_vertices with their own value.
			#pragma omp barrier
			
			//////////////////////////////////
			// SPREAD VERTICES MERGE PHASE //
			////////////////////////////////
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_spread_merge_start[ip_my_thread_num] = omp_get_wtime();
				timer_spread_merge_stop[ip_my_thread_num] = timer_spread_merge_start[ip_my_thread_num];
			#endif
			#pragma omp single
			{
				if(ip_all_spread_vertices.max_size < ip_active_vertices)
				{
					ip_all_spread_vertices.data = ip_safe_realloc(ip_all_spread_vertices.data, sizeof(IP_VERTEX_ID_TYPE) * ip_active_vertices);
					ip_all_spread_vertices.max_size = ip_active_vertices;
				}
			
				ip_all_spread_vertices.size = 0;
	
				for(int i = 0; i < ip_thread_count; i++)
				{
					if(ip_all_spread_vertices_omp[i * IP_CACHE_LINE_LENGTH].size > 0)
					{
						memmove(&ip_all_spread_vertices.data[ip_all_spread_vertices.size], ip_all_spread_vertices_omp[i * IP_CACHE_LINE_LENGTH].data, ip_all_spread_vertices_omp[i * IP_CACHE_LINE_LENGTH].size * sizeof(IP_VERTEX_ID_TYPE));
						ip_all_spread_vertices.size += ip_all_spread_vertices_omp[i * IP_CACHE_LINE_LENGTH].size;
						ip_all_spread_vertices_omp[i * IP_CACHE_LINE_LENGTH].size = 0;
					}
				}
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_spread_merge_stop[ip_my_thread_num] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_spread_merge_total[ip_my_thread_num] = timer_spread_merge_stop[ip_my_thread_num] - timer_spread_merge_start[ip_my_thread_num];
			#endif

			///////////////////////////
			// MAILBOX UPDATE PHASE //
			/////////////////////////
			// Take in account only the vertices that have been flagged as
			// spread -> that is, vertices having received a new message.
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_mailbox_update_start[ip_my_thread_num] = omp_get_wtime();
				timer_mailbox_update_stop[ip_my_thread_num] = timer_mailbox_update_start[ip_my_thread_num];
			#endif
			IP_VERTEX_ID_TYPE spread_vertex_id;
			#pragma omp for schedule(runtime)
			for(size_t i = 0; i < ip_all_spread_vertices.size; i++)
			{
				spread_vertex_id = ip_all_spread_vertices.data[i];
				temp_vertex = ip_get_vertex_by_id(spread_vertex_id);
				temp_vertex->has_message = true;
				temp_vertex->message = ip_all_externalised_structures[spread_vertex_id].message_next;
				ip_all_externalised_structures[spread_vertex_id].has_message_next = false;
				#ifdef IP_ENABLE_THREAD_PROFILING
					timer_mailbox_update_stop[ip_my_thread_num] = omp_get_wtime();
				#endif
			}
			#ifdef IP_ENABLE_THREAD_PROFILING
				timer_mailbox_update_total[ip_my_thread_num] = timer_mailbox_update_stop[ip_my_thread_num] - timer_mailbox_update_start[ip_my_thread_num];
			#endif
		
			#pragma omp single
			{
				timer_superstep_stop = omp_get_wtime();
				timer_superstep_total += (timer_superstep_stop - timer_superstep_start);
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
					printf("\n|   Merging |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3fs |", timer_spread_merge_total[i]);
					}
					printf("\n|   Mailbox |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3fs |", timer_mailbox_update_total[i]);
					}
					printf("\n|     Total |");
					for(int i = 0; i < ip_thread_count; i++)
					{
						printf(" %8.3fs |", timer_compute_total[i] + timer_spread_merge_total[i] + timer_mailbox_update_total[i]);
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

	// Free and clean program.	
	#pragma omp parallel
	{
		ip_safe_free(ip_all_spread_vertices_omp[omp_get_thread_num() * IP_CACHE_LINE_LENGTH].data);
	}
	ip_safe_free(ip_all_spread_vertices.data);

	#ifdef IP_ENABLE_THREAD_PROFILING
		free(timer_compute_start);
		free(timer_compute_stop);
		free(timer_compute_total);
		free(timer_spread_merge_start);
		free(timer_spread_merge_stop);
		free(timer_spread_merge_total);
		free(timer_mailbox_update_start);
		free(timer_mailbox_update_stop);
		free(timer_mailbox_update_total);
		free(timer_edge_count);
	#endif
	free(ip_all_externalised_structures);

	return 0;
}

void ip_vote_to_halt(struct ip_vertex_t* v)
{
	(void)(v);
}

void ip_lock_init(IP_LOCK_TYPE* lock)
{
	*lock = 0;
}

void ip_lock_acquire(IP_LOCK_TYPE* lock)
{
	int zero = 0;
	while(!atomic_compare_exchange_strong(lock, &zero, 1))
		zero = 0;
}

void ip_lock_release(IP_LOCK_TYPE* lock)
{
	atomic_store(lock, 0);
}

#endif // COMBINER_SPREAD_POSTAMBLE_H_INCLUDED
