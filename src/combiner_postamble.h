/**
 * @file combiner_postamble.h
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

#ifndef COMBINER_POSTAMBLE_H_INCLUDED
#define COMBINER_POSTAMBLE_H_INCLUDED

#include <omp.h>
#include <string.h>

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
		ip_messages_left_omp[omp_get_thread_num()]--;
		return true;
	}

	return false;
}

void ip_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE message)
{
	struct ip_vertex_t* v = ip_get_vertex_by_id(id);
	ip_lock_acquire(&v->lock);
	if(v->has_message_next)
	{
		ip_combine(&v->message_next, message);
		ip_lock_release(&v->lock);
	}
	else
	{
		v->has_message_next = true;
		v->message_next = message;
		ip_lock_release(&v->lock);
		ip_messages_left_omp[omp_get_thread_num()]++;
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
		ip_all_vertices[i].id = i - IP_ID_OFFSET;
		ip_all_vertices[i].active = true;
		ip_all_vertices[i].has_message = false;
		ip_all_vertices[i].has_message_next = false;
		#ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
			ip_all_vertices[i].out_neighbour_count = 0;
		#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
		#ifdef IP_NEEDS_OUT_NEIGHBOUR_IDS
			ip_all_vertices[i].out_neighbours = NULL;
		#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_IDS
		#ifdef IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
			ip_all_vertices[i].out_neighbour_weights = NULL;
		#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
		#ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
			ip_all_vertices[i].in_neighbours = NULL;
		#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
		#ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
			ip_all_vertices[i].in_neighbour_count = 0;
		#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
		#ifdef IP_NEEDS_IN_NEIGHBOUR_WEIGHTS
			ip_all_vertices[i].in_neighbour_weights = NULL;
		#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_WEIGHT
		ip_lock_init(&ip_all_vertices[i].lock);
	}
}

void ip_init_specific()
{
	// Initialise OpenMP variables
	#pragma omp parallel
	{
		#pragma omp master
		{
			ip_messages_left_omp = (size_t*)ip_safe_malloc(sizeof(size_t) * omp_get_num_threads());
			for(int i = 0; i < omp_get_num_threads(); i++)
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

	while(ip_active_vertices != 0 || ip_messages_left > 0)
	{
		timer_superstep_start = omp_get_wtime();
		ip_active_vertices = 0;
		#pragma omp parallel default(none) shared(ip_active_vertices, \
												  ip_messages_left, \
												  ip_messages_left_omp)
		{
			struct ip_vertex_t* temp_vertex = NULL;

			#pragma omp for reduction(+:ip_active_vertices)
			for(size_t i = 0; i < ip_get_vertices_count(); i++)
			{
				temp_vertex = ip_get_vertex_by_location(i);
				if(temp_vertex->active || ip_has_message(temp_vertex))
				{
					temp_vertex->active = true;
					ip_compute(temp_vertex);
					if(temp_vertex->active)
					{
						ip_active_vertices++;
					}
				}
			}

			#pragma omp for reduction(+:ip_messages_left)
			for(int i = 0; i < omp_get_num_threads(); i++)
			{
				ip_messages_left += ip_messages_left_omp[i];
				ip_messages_left_omp[i] = 0;
			}
		
			// Take in account the number of vertices that halted.
			// Swap the message boxes for next superstep.
			#pragma omp for
			for(size_t i = 0; i < ip_get_vertices_count(); i++)
			{
				temp_vertex = ip_get_vertex_by_location(i);
				if(temp_vertex->has_message_next)
				{
					temp_vertex->has_message = true;
					temp_vertex->message = temp_vertex->message_next;
					temp_vertex->has_message_next = false;
				}
			}
		}

		timer_superstep_stop = omp_get_wtime();
		timer_superstep_total += timer_superstep_stop - timer_superstep_start;
		printf("Superstep %zu finished in %fs; %zu active vertices and %zu messages left.\n", ip_get_superstep(), timer_superstep_stop - timer_superstep_start, ip_active_vertices, ip_messages_left);
		ip_increment_superstep();
	}
	
	printf("Total time of supersteps: %fs.\n", timer_superstep_total);

	return 0;
}

void ip_vote_to_halt(struct ip_vertex_t* v)
{
	v->active = false;
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

#endif // COMBINER_POSTAMBLE_H_INCLUDED
