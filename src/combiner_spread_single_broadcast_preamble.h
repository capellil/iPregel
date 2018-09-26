/**
 * @file combiner_spread_single_broadcast_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_SPREAD_SINGLE_BROADCAST_H_INCLUDED
#define COMBINER_SPREAD_SINGLE_BROADCAST_H_INCLUDED

// Global variables
/// This variable contains the number of active vertices at an instant t.
size_t ip_active_vertices = 0;
/// This variable contains the number of messages that have not been read yet.
size_t ip_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t* ip_messages_left_omp = NULL;
/**
 * @brief This structure holds the vertices that have a neighbour at least who
 * broadcasted.
 **/
struct ip_targets_t
{
	/// This contains the current number of targets.
	size_t size;
	/// This contains the buffer memory size. It is used for reallocation purpose.
	size_t max_size;
	/// This contains the actual target ids.
	IP_VERTEX_ID_TYPE* data;
};
/// This variable contains the targets.
struct ip_targets_t ip_all_targets;

// Prototypes
/**
 * @brief This functions add the identifier \p id to the targets.
 * @param[in] id The identifier of the new target.
 * @post \p id is added to the targets.
 **/
void ip_add_target(IP_VERTEX_ID_TYPE id);
/**
 * @brief This functions gathers and combines all the messages destined to the
 * vertex \p v.
 * @param[inout] v The vertex to update.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post All the messages destined to vertex \p v are stored in v.
 **/
void ip_fetch_broadcast_messages(struct ip_vertex_t* v);

#ifdef IP_WEIGHTED_EDGES
	/// This macro defines the minimal attributes of a vertex.
	#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
								IP_EDGE_WEIGHT_TYPE* out_edge_weights; \
								IP_VERTEX_ID_TYPE* in_neighbours; \
								IP_EDGE_WEIGHT_TYPE* in_edge_weights; \
								bool broadcast_target; \
								bool has_broadcast_message; \
								bool has_message; \
								IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
								IP_VERTEX_ID_TYPE id; \
								IP_MESSAGE_TYPE broadcast_message; \
								IP_MESSAGE_TYPE message;
#else
	/// This macro defines the minimal attributes of a vertex.
	#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
								IP_VERTEX_ID_TYPE* in_neighbours; \
								bool broadcast_target; \
								bool has_broadcast_message; \
								bool has_message; \
								IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
								IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
								IP_VERTEX_ID_TYPE id; \
								IP_MESSAGE_TYPE broadcast_message; \
								IP_MESSAGE_TYPE message;
#endif // if(n)def IP_WEIGHTED_EDGES

#endif // COMBINER_SPREAD_SINGLE_BROADCAST_PREAMBLE_H_INCLUDED
