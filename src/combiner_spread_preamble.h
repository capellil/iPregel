/**
 * @file combiner_spread_preamble.h
 * @author Ludovic Capelli
 * @brief This version is optimised for graph traversal algorithms.
 * @details This version relies on a list of vertices to run at every superstep.
 * It can provide better performance when only a small number of vertices are to
 * be executed; instead of checking all vertices if they are active, only the
 * active ones are executed.
 **/

#ifndef COMBINER_SPREAD_PREAMBLE_H_INCLUDED
#define COMBINER_SPREAD_PREAMBLE_H_INCLUDED

#include <pthread.h> 

#ifdef IP_USE_SPINLOCK
	typedef pthread_spinlock_t IP_LOCK_TYPE;	
#else
	typedef pthread_mutex_t IP_LOCK_TYPE; 
#endif // IP_USE_SPINLOCK

// Global variables
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
/// This variable contains the number of messages that have not been read yet.
size_t ip_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t* ip_messages_left_omp = NULL;
/// The number of vertices part of the current wave of vertices to execute.
size_t ip_spread_vertices_count = 0;
/// This contains all the vertices to execute next superstep.
struct ip_vertex_list_t ip_all_spread_vertices;
/// This contains the vertices that threads found to be executed next superstep.
struct ip_vertex_list_t* ip_all_spread_vertices_omp = NULL;
/// This structure defines the structure of a vertex.
struct ip_vertex_t
{
	/// Contains the identifiers of out-neighbours
	IP_VERTEX_ID_TYPE* out_neighbours;
	#ifdef IP_WEIGHTED_EDGES
		/// Contains the weights for out-going edges
		IP_EDGE_WEIGHT_TYPE* out_edge_weights;
	#endif // ifdef IP_WEIGHTED_EDGES
	#if !defined(IP_UNUSED_IN_NEIGHBOURS) && !defined(IP_UNUSED_IN_NEIGHBOUR_IDS)
		/// Contains the identifiers of in-neighbours
		IP_VERTEX_ID_TYPE* in_neighbours;
		#ifdef IP_WEIGHTED_EDGES
			/// Contains the weights for in-going edges
			IP_EDGE_WEIGHT_TYPE* in_edge_weights;
		#endif // IP_WEIGHTED_EDGES
	#endif // if !defined(IP_UNUSED_IN_NEIGHBOURS) && !defined(IP_UNUSED_NEIGHBOUR_IDS)
	/// Indicates whether the vertex is active or not
	bool active;
	/// Indicates whether the vertex has received messages from last superstep
	bool has_message;
	/// Indicates whether the vertex has received messages from current superstep so far
	bool has_message_next;
	/// Contains the number of out-neighbours
	IP_NEIGHBOUR_COUNT_TYPE out_neighbour_count;
	#ifndef IP_UNUSED_IN_NEIGHBOURS
		/// Contains the number of in-neighbours
		IP_NEIGHBOUR_COUNT_TYPE in_neighbour_count;
	#endif // IP_UNUSED_IN_NEIGHBOURS
	/// The lock used for mailbox thread-safe accesses
	IP_LOCK_TYPE lock;
	/// The vertex identifier
	IP_VERTEX_ID_TYPE id;
	/// Contains the combined message made from messages received from last superstep
	IP_MESSAGE_TYPE message;
	/// Contains the combined message made from message received from current superstep so far
	IP_MESSAGE_TYPE message_next;
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
