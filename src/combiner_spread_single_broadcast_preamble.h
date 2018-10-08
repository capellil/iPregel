/**
 * @file combiner_spread_single_broadcast_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef COMBINER_SPREAD_SINGLE_BROADCAST_H_INCLUDED
#define COMBINER_SPREAD_SINGLE_BROADCAST_H_INCLUDED

// Global variables
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
/// This structure defines the structure of a vertex.
struct ip_vertex_t
{
	IP_VERTEX_ID_TYPE* in_neighbours;
	#ifdef IP_WEIGHTED_EDGES
		IP_EDGE_WEIGHT_TYPE* in_edge_weights;
	#endif // IP_WEIGHTED_EDGES
	IP_VERTEX_ID_TYPE* out_neighbours;
	#ifdef IP_WEIGHTED_EDGES
		IP_EDGE_WEIGHT_TYPE* out_edge_weights;
	#endif // IP_WEIGHTED_EDGES
	bool broadcast_target;
	bool has_broadcast_message;
	bool has_message;
	IP_NEIGHBOUR_COUNT_TYPE in_neighbour_count;
	IP_NEIGHBOUR_COUNT_TYPE out_neighbour_count;
	IP_VERTEX_ID_TYPE id;
	IP_MESSAGE_TYPE broadcast_message;
	IP_MESSAGE_TYPE message;
	IP_VALUE_TYPE value;
};

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

#endif // COMBINER_SPREAD_SINGLE_BROADCAST_PREAMBLE_H_INCLUDED
