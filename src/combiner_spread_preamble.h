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

// Global variables
/// This structure holds a list of vertex identifiers.
struct vertex_list_t
{
	/// The size of the memory buffer. It is used for reallocation purpose.
	size_t max_size;
	/// The number of identifiers currently stored.
	size_t size;
	/// The actual identifiers.
	VERTEX_ID* data;
};
/// This variable contains the number of active vertices at an instant t.
unsigned int active_vertices = 0;
/// This variable contains the number of messages that have not been read yet.
unsigned int messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
unsigned int messages_left_omp[OMP_NUM_THREADS] = {0};
/// The number of vertices part of the current wave of vertices to execute.
unsigned int spread_vertices_count = 0;
/// This variable is used for multithread reduction into spread_vertices_count.
unsigned int spread_vertices_count_omp[OMP_NUM_THREADS] = {0};
/// This contains all the vertices to execute next superstep.
struct vertex_list_t all_spread_vertices;
/// This contains the vertices that threads found to be executed next superstep.
struct vertex_list_t all_spread_vertices_omp[OMP_NUM_THREADS];
/// This variable contains the current superstep number. It is 0-indexed.
unsigned int superstep = 0;
/// This variable contains the total number of vertices.
unsigned int vertices_count = 0;
/// This variable contains all the vertices.
struct vertex_t* all_vertices = NULL;

/**
 * @brief This function adds the given vertex to the list of vertices to execute
 * at next superstep.
 * @param[in] id The identifier of the vertex to executed next superstep.
 * @post The vertex identifier by \p id will be executed at next superstep.
 **/
void add_spread_vertex(VERTEX_ID id);

#ifdef USE_SPIN_LOCK
	/// This macro defines the type of lock used.
	#define MY_PREGEL_LOCKTYPE pthread_spinlock_t
	/// This macro defines how the lock can be initialised.
	#define MY_PREGEL_LOCK_INIT(X) pthread_spin_init(X, PTHREAD_PROCESS_PRIVATE)
	/// This macro defines how the lock can be locked.
	#define MY_PREGEL_LOCK(X) pthread_spin_lock(X)
	/// This macro defines how the lock can be unlocked.
	#define MY_PREGEL_UNLOCK(X) pthread_spin_unlock(X)
	/// This macro defines the minimal attributes of a vertex.
	#define VERTEX_STRUCTURE VERTEX_ID* out_neighbours; \
							 VERTEX_ID* in_neighbours; \
 							 bool active; \
							 bool has_message; \
							 bool has_message_next; \
							 unsigned int out_neighbours_count; \
							 unsigned int in_neighbours_count; \
							 pthread_spinlock_t lock; \
							 VERTEX_ID id; \
							 MESSAGE_TYPE message; \
							 MESSAGE_TYPE message_next;
#else // ifndef USE_SPIN_LOCK
	/// This macro defines the type of lock used.
	#define MY_PREGEL_LOCKTYPE pthread_mutex_t
	/// This macro defines how the lock can be initialised.
	#define MY_PREGEL_LOCK_INIT(X) pthread_mutex_init(X, NULL)
	/// This macro defines how the lock can be locked.
	#define MY_PREGEL_LOCK(X) pthread_mutex_lock(X)
	/// This macro defines how the lock can be unlocked.
	#define MY_PREGEL_UNLOCK(X) pthread_mutex_unlock(X)
	/// This macro defines the minimal attributes of a vertex.
	#define VERTEX_STRUCTURE VERTEX_ID* out_neighbours; \
							 VERTEX_ID* in_neighbours; \
 							 bool active; \
							 bool has_message; \
							 bool has_message_next; \
							 unsigned int out_neighbours_count; \
							 unsigned int in_neighbours_count; \
							 pthread_mutex_t lock; \
							 VERTEX_ID id; \
							 MESSAGE_TYPE message; \
							 MESSAGE_TYPE message_next;
#endif // if(n)def USE_SPIN_LOCK

#endif // COMBINER_SPREAD_PREAMBLE_H_INCLUDED
