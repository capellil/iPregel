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
/// This variable contains the number of active vertices at an instant t.
size_t ip_active_vertices = 0;
/// This variable contains the number of messages that have not been read yet.
size_t ip_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t ip_messages_left_omp[OMP_NUM_THREADS] = {0};

// Prototypes
/**
 * @brief This functions gathers and combines all the messages destined to the
 * vertex \p v.
 * @param[inout] v The vertex to update.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post All the messages destined to vertex \p v are stored in v.
 **/
void ip_fetch_broadcast_messages(struct ip_vertex_t* v);

#ifdef IP_WEIGHTED_EDGES
	#ifdef IP_UNUSED_OUT_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* in_neighbours; \
									IP_EDGE_WEIGHT_TYPE* in_edge_weights; \
									bool active; \
									bool has_broadcast_message; \
									bool has_message; \
									IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
									IP_VERTEX_ID_TYPE id; \
									IP_MESSAGE_TYPE broadcast_message; \
									IP_MESSAGE_TYPE message;
	#else // ifndef IP_UNUSED_OUT_NEIGHBOURS
		#ifdef IP_UNUSED_OUT_NEIGHBOUR_IDS
			/// This macro defines the minimal attributes of a vertex.
			#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* in_neighbours; \
										IP_EDGE_WEIGHT_TYPE* in_edge_weights; \
										bool active; \
										bool has_broadcast_message; \
										bool has_message; \
										IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
										IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
										IP_VERTEX_ID_TYPE id; \
										IP_MESSAGE_TYPE broadcast_message; \
										IP_MESSAGE_TYPE message;
		#else // ifndef IP_UNUSED_OUT_NEIGHBOUR_IDS
			/// This macro defines the minimal attributes of a vertex.
			#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
										IP_EDGE_WEIGHT_TYPE* out_edge_weights; \
										IP_VERTEX_ID_TYPE* in_neighbours; \
										IP_EDGE_WEIGHT_TYPE* in_edge_weights; \
										bool active; \
										bool has_broadcast_message; \
										bool has_message; \
										IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
										IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
										IP_VERTEX_ID_TYPE id; \
										IP_MESSAGE_TYPE broadcast_message; \
										IP_MESSAGE_TYPE message;
		#endif // if(n)def IP_UNUSED_OUT_NEIGHBOUR_IDS
	#endif // if(n)def IP_UNUSED_OUT_NEIGHBOURS
#else // ifndef IP_WEIGHTED_EDGES
	#ifdef IP_UNUSED_OUT_NEIGHBOURS
		/// This macro defines the minimal attributes of a vertex.
		#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* in_neighbours; \
									bool active; \
									bool has_broadcast_message; \
									bool has_message; \
									IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
									IP_VERTEX_ID_TYPE id; \
									IP_MESSAGE_TYPE broadcast_message; \
									IP_MESSAGE_TYPE message;
	#else // ifndef IP_UNUSED_OUT_NEIGHBOURS
		#ifdef IP_UNUSED_OUT_NEIGHBOUR_IDS
			/// This macro defines the minimal attributes of a vertex.
			#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* in_neighbours; \
										bool active; \
										bool has_broadcast_message; \
										bool has_message; \
										IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
										IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
										IP_VERTEX_ID_TYPE id; \
										IP_MESSAGE_TYPE broadcast_message; \
										IP_MESSAGE_TYPE message;
		#else // ifndef IP_UNUSED_OUT_NEIGHBOUR_IDS
			/// This macro defines the minimal attributes of a vertex.
			#define IP_VERTEX_STRUCTURE IP_VERTEX_ID_TYPE* out_neighbours; \
										IP_VERTEX_ID_TYPE* in_neighbours; \
										bool active; \
										bool has_broadcast_message; \
										bool has_message; \
										IP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
										IP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
										IP_VERTEX_ID_TYPE id; \
										IP_MESSAGE_TYPE broadcast_message; \
										IP_MESSAGE_TYPE message;
		#endif // if(n)def IP_UNUSED_OUT_NEIGHBOUR_IDS
	#endif // if(n)def IP_UNUSED_OUT_NEIGHBOURS
#endif // if(n)def IP_WEIGHTED_EDGES

#endif // SINGLE_BROADCAST_PREAMBLE_H_INCLUDED
