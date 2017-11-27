/**
 * @file combiner_spread_single_broadcast_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_SPREAD_SINGLE_BROADCAST_H_INCLUDED
#define COMBINER_SPREAD_SINGLE_BROADCAST_H_INCLUDED

// Global variables
/// This variable contains the number of active vertices at an instant t.
unsigned int active_vertices = 0;
/// This variable contains the number of messages that have not been read yet.
unsigned int messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
unsigned int messages_left_omp[OMP_NUM_THREADS] = {0};
/// This variable contains the current superstep number. It is 0-indexed.
unsigned int superstep = 0;
/// This variable contains the total number of vertices.
unsigned int vertices_count = 0;
/// This variable contains all the vertices.
struct vertex_t* all_vertices = NULL;

// Prototypes
/**
 * @brief This functions gathers and combines all the messages destined to the
 * vertex \p v.
 * @param[inout] v The vertex to update.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post All the messages destined to vertex \p v are stored in v.
 **/
void fetch_broadcast_messages(struct vertex_t* v);

/// This macro defines the minimal attributes of a vertex.
#define VERTEX_STRUCTURE VERTEX_ID* out_neighbours; \
						 VERTEX_ID* in_neighbours; \
						 bool active; \
						 bool broadcast_target; \
						 bool has_broadcast_message; \
						 bool has_message; \
						 unsigned int out_neighbours_count; \
						 unsigned int in_neighbours_count; \
						 VERTEX_ID id; \
						 MESSAGE_TYPE broadcast_message; \
						 MESSAGE_TYPE message;

#endif // COMBINER_SPREAD_SINGLE_BROADCAST_PREAMBLE_H_INCLUDED
