/**
 * @file combiner_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_PREAMBLE_H_INCLUDED
#define COMBINER_PREAMBLE_H_INCLUDED

#include <pthread.h> 

#ifdef IP_USE_SPINLOCK
	typedef pthread_spinlock_t IP_LOCK_TYPE;	
#else
	typedef pthread_mutex_t IP_LOCK_TYPE; 
#endif // IP_USE_SPINLOCK

// Global variables
/// This variable contains the number of messages that have not been read yet.
size_t ip_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t* ip_messages_left_omp = NULL;
/// This structure defines the structure of a vertex.
struct ip_vertex_t
{
	IP_VERTEX_ID_TYPE* out_neighbours;
	#ifdef IP_WEIGHTED_EDGES
		IP_EDGE_WEIGHT_TYPE* out_edge_weights;
	#endif // ifdef IP_WEIGHTED_EDGES
	#if !defined(IP_UNUSED_IN_NEIGHBOURS) && !defined(IP_UNUSED_IN_NEIGHBOUR_IDS)
		IP_VERTEX_ID_TYPE* in_neighbours;
		#ifdef IP_WEIGHTED_EDGES
			IP_EDGE_WEIGHT_TYPE* in_edge_weights;
		#endif // IP_WEIGHTED_EDGES
	#endif // if !defined(IP_UNUSED_IN_NEIGHBOURS) && !defined(IP_UNUSED_NEIGHBOUR_IDS)
	bool active;
	bool has_message;
	bool has_message_next;
	IP_NEIGHBOUR_COUNT_TYPE out_neighbour_count;
	#ifndef IP_UNUSED_IN_NEIGHBOURS
		IP_NEIGHBOUR_COUNT_TYPE in_neighbour_count;
	#endif // IP_UNUSED_IN_NEIGHBOURS
	IP_LOCK_TYPE lock;
	IP_VERTEX_ID_TYPE id;
	IP_MESSAGE_TYPE message;
	IP_MESSAGE_TYPE message_next;
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
