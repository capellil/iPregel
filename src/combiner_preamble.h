/**
 * @file combiner_preamble.h
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

#ifndef COMBINER_PREAMBLE_H_INCLUDED
#define COMBINER_PREAMBLE_H_INCLUDED

#ifndef IP_NEEDS_OUT_NEIGHBOUR_IDS
	#define IP_NEEDS_OUT_NEIGHBOUR_IDS
#endif // ifndef IP_NEEDS_OUT_NEIGHBOUR_IDS

#ifndef IP_NEEDS_OUT_NEIGHBOUR_COUNT
	#define IP_NEEDS_OUT_NEIGHBOUR_COUNT
#endif // ifndef IP_NEEDS_OUT_NEIGHBOUR_COUNT

#include <stdatomic.h> 

// Global variables
/// The data structure representing a lock.
typedef volatile atomic_int IP_LOCK_TYPE;	
/// This structure defines the structure of a vertex.
struct ip_vertex_t
{
	#ifdef IP_NEEDS_OUT_NEIGHBOUR_IDS
		/// Contains the identifiers of the out-neighbours
		IP_VERTEX_ID_TYPE* out_neighbours;
	#endif // IP_NEEDS_OUT_NEIGHBOUR_IDS
	#ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
		/// Contains the identifiers of the in-neighbours
		IP_VERTEX_ID_TYPE* in_neighbours;
	#endif // ifdef IP_NEEDS_IN_NEIGHBOUR_IDS
	#ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
		/// Contains the number of out-neighbours
		IP_NEIGHBOUR_COUNT_TYPE out_neighbour_count;
	#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_COUNT
	#ifdef IP_NEEDS_IN_NEIGHBOUR_COUNT
		/// Contains the number of in-neighbours
		IP_NEIGHBOUR_COUNT_TYPE in_neighbour_count;
	#endif // IP_NEEDS_IN_NEIGHBOUR_COUNT
	#ifdef IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
		/// Contains the weights of out-edges
		IP_EDGE_WEIGHT_TYPE* out_neighbour_weights;
	#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
	#ifdef IP_NEEDS_IN_NEIGHBOUR_WEIGHTS
		/// Contains the weights of the in-neighbours
		IP_EDGE_WEIGHT_TYPE* in_neighbour_weights;
	#endif // IP_NEEDS_IN_NEIGHBOUR_WEIGHTS
	/// Contains the vertex status
	bool active;
	/// Indicates whether the vertex has received messages during the previous superstep
	bool has_message;
	/// Indicates whether the vertex has received message during the current superstep so far
	atomic_bool has_message_next;
	/// Mailbox lock
	IP_LOCK_TYPE lock;
	/// Contains the vertex identifier
	IP_VERTEX_ID_TYPE id;
	/// Contains the combined message resulting from messages received during previous superstep
	IP_MESSAGE_TYPE message;
	/// Contains the combined message resulting from messages received during current superstep so far
	IP_MESSAGE_TYPE message_next;
	/// Contains the user-defined value
	IP_VALUE_TYPE value;
};

/**
 * @brief This function initialises the lock \p lock.
 * @param[in] lock The lock to initialise.
 **/
void ip_lock_init(IP_LOCK_TYPE* lock);
/**
 * @brief This function acquires the lock \p lock.
 * @param[in] lock The lock to acquire.
 **/
void ip_lock_acquire(IP_LOCK_TYPE* lock);
/**
 * @brief This function releases the lock \p lock.
 * @param[in] lock The lock to release.
 **/
void ip_lock_release(IP_LOCK_TYPE* lock);

#endif // COMBINER_PREAMBLE_H_INCLUDED
