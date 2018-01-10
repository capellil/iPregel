/**
 * @file combiner_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_PREAMBLE_H_INCLUDED
#define COMBINER_PREAMBLE_H_INCLUDED

#include <pthread.h> 

// Global variables
/// This variable contains the number of active vertices at an instant t.
size_t mp_active_vertices = 0;
/// This variable contains the number of messages that have not been read yet.
size_t mp_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t mp_messages_left_omp[OMP_NUM_THREADS] = {0};

#ifdef MP_USE_SPINLOCK
	/// This macro defines the type of lock used.
	#define MP_LOCKTYPE pthread_spinlock_t
	/// This macro defines how the lock can be initialised.
	#define MP_LOCK_INIT(X) pthread_spin_init(X, PTHREAD_PROCESS_PRIVATE)
	/// This macro defines how the lock can be locked.
	#define MP_LOCK(X) pthread_spin_lock(X)
	/// This macro defines how the lock can be unlocked.
	#define MP_UNLOCK(X) pthread_spin_unlock(X)
	#ifdef MP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define MP_VERTEX_STRUCTURE MP_VERTEX_ID_TYPE* out_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	MP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	pthread_spinlock_t lock; \
								 	MP_VERTEX_ID_TYPE id; \
								 	MP_MESSAGE_TYPE message; \
								 	MP_MESSAGE_TYPE message_next;
	#else // ifndef MP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define MP_VERTEX_STRUCTURE MP_VERTEX_ID_TYPE* out_neighbours; \
								 	MP_VERTEX_ID_TYPE* in_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	MP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	MP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
								 	pthread_spinlock_t lock; \
								 	MP_VERTEX_ID_TYPE id; \
								 	MP_MESSAGE_TYPE message; \
								 	MP_MESSAGE_TYPE message_next;
	#endif // if(n)def MP_UNUSED_IN_NEIGHBOURS
#else // ifndef MP_USE_SPINLOCK
	/// This macro defines the type of lock used.
	#define MP_LOCKTYPE pthread_mutex_t
	/// This macro defines how the lock can be initialised.
	#define MP_LOCK_INIT(X) pthread_mutex_init(X, NULL)
	/// This macro defines how the lock can be locked.
	#define MP_LOCK(X) pthread_mutex_lock(X)
	/// This macro defines how the lock can be unlocked.
	#define MP_UNLOCK(X) pthread_mutex_unlock(X)
	#ifdef MP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define MP_VERTEX_STRUCTURE MP_VERTEX_ID_TYPE* out_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	MP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	pthread_mutex_t lock; \
								 	MP_VERTEX_ID_TYPE id; \
								 	MP_MESSAGE_TYPE message; \
								 	MP_MESSAGE_TYPE message_next;
	#else // ifndef MP_UNUSED_IN_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define MP_VERTEX_STRUCTURE MP_VERTEX_ID_TYPE* out_neighbours; \
								 	MP_VERTEX_ID_TYPE* in_neighbours; \
 								 	bool active; \
								 	bool has_message; \
								 	bool has_message_next; \
								 	MP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								 	MP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
								 	pthread_mutex_t lock; \
								 	MP_VERTEX_ID_TYPE id; \
								 	MP_MESSAGE_TYPE message; \
								 	MP_MESSAGE_TYPE message_next;
	#endif // if(n)def MP_UNUSED_IN_NEIGHBOURS
#endif // if(n)def MP_USE_SPINLOCK

#endif // COMBINER_PREAMBLE_H_INCLUDED
