/**
 * @file my_pregel_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef X_PREAMBLE_H_INCLUDED
#define X_PREAMBLE_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

/// Incomplete declaration to not raise warnings.
struct vertex_t;

// Prototypes that are valid across all versions.
/**
 * @brief This function tells whether the vertex \p v has a message to read.
 * 
 * This function just checks if the vertex \p v has a message to read or not, it
 * does not consume the message.
 * @param[in] v The vertex to process.
 * @retval true The vertex has at least one message unread.
 * @retval false The vertex has no message left to read.
 **/
bool has_message(struct vertex_t* v);
/**
 * @brief This function consumers the next message in vertex \p v mailbox.
 * 
 * This function consumes the next message in vertex \p v mailbox and stores it
 * in \p message. Prior to consuming the message, this function checks if the 
 * vertex \p v actually has at least one message to read. The result of this
 * check is what this functions returns, which allows for this function to be
 * used in a loop condition statement.
 * @param[inout] v The vertex to process.
 * @param[out] message The variable in which to store the message consumed.
 * @retval true The vertex \p v had a message to read; this message has been 
 * consumed and is stored in \p message.
 * @retval false The vertex \p had no message left to read. The message \p
 * message is left untouched.
 * @pre v points to a memory area already allocated for a vertex.
 * @pre message points to a memory area already allocated for a message.
 **/
bool get_next_message(struct vertex_t* v, MESSAGE_TYPE* message);
/**
 * @brief This function builds a vertex \p v from its representation in file \p
 * f.
 * 
 * Since this function is implemented by the end-user, post-conditions cannot be
 * asserted by the author of my_pregel.
 * @param[in] f The file to read from.
 * @param[in] v The vertex in which to store the vertex built from \p f.
 * @pre f points to a file successfully open for reading.
 * @pre v points to a memory area already allocated for a vertex.
 **/
void deserialise_vertex(FILE* f, struct vertex_t* v);
/**
 * @brief This function writes in a file the serialised representation of a
 * vertex.
 * @param[out] f The file into which write.
 * @param[in] v The vertex to serialise.
 **/
void serialise_vertex(FILE* f, struct vertex_t* v);
/**
 * @brief This function sends the message \p message to the vertex identified 
 * by \p id.
 * @param[in] id The identifier of the destination vertex.
 * @param[in] message The message to send.
 * @pre id is an existing vertex identifier.
 * @post The message is delivered to the destination vertex. If a combiner is
 * used, this message may be ignored.
 **/
void send_message(VERTEX_ID id, MESSAGE_TYPE message);
/**
 * @brief This function sends the message \p message to all neighbours of the
 * vertex \p v.
 * @param[out] v The vertex broadcasting.
 * @param[in] message The message to broadcast.
 **/
void broadcast(struct vertex_t* v, MESSAGE_TYPE message);
/**
 * @brief This function halts the vertex \p v.
 * @param[out] v The vertex to halt.
 * @pre v points to a memory area already allocated for a vertex.
 * @post The vertex \p v is inactive.
 **/
void vote_to_halt(struct vertex_t* v);
/**
 * @brief This function writes the serialised representation of all vertices
 * in the file \p f.
 * @param[out] f The file to dump into.
 **/
void dump(FILE* f);
/**
 * @brief This function executes a malloc and checks the memory area was
 * successfully allocated, otherwise exits the program.
 * @param[in] size_to_malloc The size to allocate, in bytes.
 * @return A pointer on the memory area allocated.
 **/
void* safe_malloc(size_t size_to_malloc);
/**
 * @brief This function executes a realloc and checks that it succeeded, 
 * otherwise exits the program.
 * @param[in] ptr A pointer on the memory area to reallocate.
 * @param[in] size_to_realloc The size of reallocate.
 * @return A pointer on the memory area reallocated.
 **/
void* safe_realloc(void* ptr, size_t size_to_realloc);

// User-defined functions
/**
 * @brief This function combines two messages into one.
 * 
 * This function being defined by the user, not all post-conditions cannot be
 * defined by the author. There is one constraint however: the combined value, 
 * if any, must be stored in \p a.
 * @param[inout] a The reference message.
 * @param[in] b The message to compare.
 * @post If a combination happened, the message \a contains the 
 * combined value.
 **/
extern void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b);
/**
 * @brief This function performs the actual superstep calculations of a vertex.
 * 
 * This function must be defined by the user.
 * @param[inout] v The vertex to process.
 **/
extern void compute(struct vertex_t* v);
/**
 * @brief This function initialises the environment and architecture of 
 * my_pregel.
 * @param[inout] f The file to read from.
 * @param[in] number_of_vertices The number of vertices to load from the file.
 * @post f must point to a file, already open.
 * @retval 0 Success.
 **/
extern int init(FILE* f, unsigned int number_of_vertices);
/**
 * @brief This function acts as the start point of the my_pregel simulation.
 * @return The error code.
 * @retval 0 Success.
 **/
extern int run();

#ifdef USE_COMBINER
	#ifdef SINGLE_BROADCAST
		#include "single_broadcast_preamble.h"
	#else // ifndef SINGLE_BROADCAST
		#include "combiner_preamble.h"
	#endif // if(n)def SINGLE_BROADCAST
#else // ifndef USE_COMBINER
	#include "no_combiner_preamble.h"
#endif // if(n)def USE_COMBINER
	
#endif // MY_PREAMBLE_H_INCLUDED
