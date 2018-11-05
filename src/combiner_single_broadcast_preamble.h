/**
 * @file combiner_single_broadcast_preamble.h
 * @author Ludovic Capelli
 * @brief This file is the header optimised for problems with a single
 * broadcast only.
 * @details What is referred to as "single broadcast only" is the fact that ALL
 * vertices, at ANY superstep, if they have to communicate they do a broadcast
 * and no other communication.
 * This version is entirely redesigned to exploit this broadcast; since all
 * vertices send the same value to all neighbours; these neighbours can come and
 * get the value at the end of every superstep instead of sending them. This
 * change makes a big difference since there is no longer data races as the
 * neighbours will siiply read the value broadcasted, compared to sending a 
 * message to one's neighbours using mutexes protecting their mailboxes.
 **/

#ifndef SINGLE_BROADCAST_H_INCLUDED
#define SINGLE_BROADCAST_H_INCLUDED

#ifndef IP_NEEDS_IN_NEIGHBOUR_IDS
	#define IP_NEEDS_IN_NEIGHBOUR_IDS
#endif // ifndef IP_NEEDS_IN_NEIGHBOUR_IDS

#ifndef IP_NEEDS_IN_NEIGHBOUR_COUNT
	#define IP_NEEDS_IN_NEIGHBOUR_COUNT
#endif // ifndef IP_NEEDS_IN_NEIGHBOUR_COUNT

// Global variables
/// This variable contains the number of messages that have not been read yet.
size_t ip_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t* ip_messages_left_omp = NULL;
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
	/// Indicates whether the vertex has a message for broadcast
	bool has_broadcast_message;
	/// Indicate whether the vertex has received messages from last superstep
	bool has_message;
	/// Contains the vertex identifier
	IP_VERTEX_ID_TYPE id;
	/// Contains the message to broadcast
	IP_MESSAGE_TYPE broadcast_message;
	/// Contains the combined message made from messages received from last superstep
	IP_MESSAGE_TYPE message;
	/// Contains the user-defined value
	IP_VALUE_TYPE value;
};

// Prototypes
/**
 * @brief This functions gathers and combines all the messages destined to the
 * vertex \p v.
 * @param[inout] v The vertex to update.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post All the messages destined to vertex \p v are stored in v.
 **/
void ip_fetch_broadcast_messages(struct ip_vertex_t* v);

#endif // SINGLE_BROADCAST_PREAMBLE_H_INCLUDED
