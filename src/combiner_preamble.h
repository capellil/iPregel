/**
 * @file combiner_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_PREAMBLE_H_INCLUDED
#define COMBINER_PREAMBLE_H_INCLUDED

#include <pthread.h> 

// Global variables
/// This variable contains the number of active vertices at an instant t.
size_t ip_active_vertices = 0;
/// This variable contains the number of messages that have not been read yet.
size_t ip_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t ip_messages_left_omp[OMP_NUM_THREADS] = {0};

#ifdef IP_USE_SPINLOCK
	/// This macro defines the type of lock used.
	#define IP_LOCKTYPE pthread_spinlock_t
	/// This macro defines how the lock can be initialised.
	#define IP_LOCK_INIT(X) pthread_spin_init(X, PTHREAD_PROCESS_PRIVATE)
	/// This macro defines how the lock can be locked.
	#define IP_LOCK(X) pthread_spin_lock(X)
	/// This macro defines how the lock can be unlocked.
	#define IP_UNLOCK(X) pthread_spin_unlock(X)
	#ifdef IP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	pthread_spinlock_t lock; \
								 	IP_VERTEX_ID_TYPE id; \
								 	IP_MESSAGE_TYPE message; \
								 	IP_MESSAGE_TYPE message_next;
	#else // ifndef IP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
								 	IP_VERTEX_ID_TYPE* in_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
								 	pthread_spinlock_t lock; \
								 	IP_VERTEX_ID_TYPE id; \
								 	IP_MESSAGE_TYPE message; \
								 	IP_MESSAGE_TYPE message_next;
	#endif // if(n)def IP_UNUSED_IN_NEIGHBOURS
#else // ifndef IP_USE_SPINLOCK
	/// This macro defines the type of lock used.
	#define IP_LOCKTYPE pthread_mutex_t
	/// This macro defines how the lock can be initialised.
	#define IP_LOCK_INIT(X) pthread_mutex_init(X, NULL)
	/// This macro defines how the lock can be locked.
	#define IP_LOCK(X) pthread_mutex_lock(X)
	/// This macro defines how the lock can be unlocked.
	#define IP_UNLOCK(X) pthread_mutex_unlock(X)
	#ifdef IP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	pthread_mutex_t lock; \
								 	IP_VERTEX_ID_TYPE id; \
								 	IP_MESSAGE_TYPE message; \
								 	IP_MESSAGE_TYPE message_next;
	#else // ifndef IP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
								 	IP_VERTEX_ID_TYPE* in_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
								 	pthread_mutex_t lock; \
								 	IP_VERTEX_ID_TYPE id; \
								 	IP_MESSAGE_TYPE message; \
								 	IP_MESSAGE_TYPE message_next;
	#endif // if(n)def IP_UNUSED_IN_NEIGHBOURS
#endif // if(n)def IP_USE_SPINLOCK

#endif // COMBINER_PREAMBLE_H_INCLUDED
