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

// Global variables
/// This variable contains the number of messages that have not been read yet.
size_t ip_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t* ip_messages_left_omp = NULL;
/// This structure defines the structure of a vertex.
struct ip_vertex_t
{
	IP_VERTEX_ID_TYPE* in_neighbours;
	#ifdef IP_WEIGHTED_EDGES
		IP_EDGE_WEIGHT_TYPE* in_edge_weights;
	#endif // ifdef IP_WEIGHTED_EDGES
	#if !defined(IP_UNUSED_OUT_NEIGHBOURS) && !defined(IP_UNUSED_OUT_NEIGHBOUR_IDS)
		IP_VERTEX_ID_TYPE* out_neighbours;
		#ifdef IP_WEIGHTED_EDGES
			IP_EDGE_WEIGHT_TYPE* out_edge_weights;
		#endif // IP_WEIGHTED_EDGES
	#endif // if !defined(IP_UNUSED_IN_NEIGHBOURS) && !defined(IP_UNUSED_NEIGHBOUR_IDS)
	bool active;
	bool has_broadcast_message;
	bool has_message;
	IP_NEIGHBOUR_COUNT_TYPE in_neighbour_count;
	#ifndef IP_UNUSED_OUT_NEIGHBOURS
		IP_NEIGHBOUR_COUNT_TYPE out_neighbour_count;
	#endif // IP_UNUSED_IN_NEIGHBOURS
	IP_VERTEX_ID_TYPE id;
	IP_MESSAGE_TYPE broadcast_message;
	IP_MESSAGE_TYPE message;
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
