/**
 * @file combiner_spread_preamble.h
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
 * @brief This version is optimised for graph traversal algorithms.
 * @details This version relies on a list of vertices to run at every superstep.
 * It can provide better performance when only a small number of vertices are to
 * be executed; instead of checking all vertices if they are active, only the
 * active ones are executed.
 **/

#ifndef COMBINER_SPREAD_PREAMBLE_H_INCLUDED
#define COMBINER_SPREAD_PREAMBLE_H_INCLUDED

#include <stdatomic.h> 

#ifndef IP_NEEDS_OUT_NEIGHBOUR_IDS
	#define IP_NEEDS_OUT_NEIGHBOUR_IDS
#endif // ifndef IP_NEEDS_OUT_NEIGHBOUR_IDS

#ifndef IP_NEEDS_OUT_NEIGHBOUR_COUNT
	#define IP_NEEDS_OUT_NEIGHBOUR_COUNT
#endif // ifndef IP_NEEDS_OUT_NEIGHBOUR_COUNT

// Global variables
/// The data type used to implement locks.
typedef volatile atomic_int IP_LOCK_TYPE;	
/// This structure holds a list of vertex identifiers.
struct ip_vertex_list_t
{
	/// The size of the memory buffer. It is used for reallocation purpose.
	size_t max_size;
	/// The number of identifiers currently stored.
	size_t size;
	/// The actual identifiers.
	IP_VERTEX_ID_TYPE* data;
};
/// The number of vertices part of the current wave of vertices to execute.
size_t ip_spread_vertices_count = 0;
/// This contains all the vertices to execute next superstep.
struct ip_vertex_list_t ip_all_spread_vertices;
/// This contains the vertices that threads found to be executed next superstep.
struct ip_vertex_list_t* ip_all_spread_vertices_omp = NULL;
/// Contains active broadcast attributes
struct ip_externalised_structure_t
{
	/// Indicates whether the vertex has received messages from current superstep so far
	atomic_bool has_message_next;
	/// The lock used for mailbox thread-safe accesses
	IP_LOCK_TYPE lock;
	/// Contains the combined message made from message received from current superstep so far
	IP_MESSAGE_TYPE message_next;
};
/// Contains the active broadcast attributes for all vertices
struct ip_externalised_structure_t* ip_all_externalised_structures = NULL;
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
	#endif // IP_UNUSED_IN_NEIGHBOURS
	#ifdef IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
		/// Contains the weights of out-edges
		IP_EDGE_WEIGHT_TYPE* out_neighbour_weights;
	#endif // ifdef IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS
	#ifdef IP_NEEDS_IN_NEIGHBOUR_WEIGHTS
		/// Contains the weights of the in-neighbours
		IP_EDGE_WEIGHT_TYPE* in_neighbour_weights;
	#endif // IP_WEIGHTED_EDGES
	/// Indicates whether the vertex is active or not
	bool active;
	/// Indicates whether the vertex has received messages from last superstep
	bool has_message;
	/// The vertex identifier
	IP_VERTEX_ID_TYPE id;
	/// Contains the combined message made from messages received from last superstep
	IP_MESSAGE_TYPE message;
	/// Contains the user-defined value
	IP_VALUE_TYPE value;
};

/**
 * @brief This function adds the given vertex to the list of vertices to execute
 * at next superstep.
 * @param[in] id The identifier of the vertex to executed next superstep.
 * @post The vertex identifier by \p id will be executed at next superstep.
 **/
void ip_add_spread_vertex(IP_VERTEX_ID_TYPE id);
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

#endif // COMBINER_SPREAD_PREAMBLE_H_INCLUDED
