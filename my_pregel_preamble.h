/**
 * @file my_pregel_preamble.h
 * @author Ludovic Capelli
 * @brief This file acts as a manager of all the versions available.
 * @details The program my_pregel contains several versions, each optimised for
 * a given set of assumptions. This file is in charge of selecting the right
 * version according to the assumptions passed via compiler flags.
 * Each optimised version may use additional functions of their own for specific
 * purpose. However, all versions provide the user with a common set of
 * functions that form the overall user interface. These functions are described
 * by the prototypes in this file. Once again, the functions that are defined in
 * the optimised versions are not meant for the user.
 * This file must be included AFTER the user defined the types VERTEX_ID_TYPE
 * and MESSAGE_TYPE.
 **/

#ifndef X_PREAMBLE_H_INCLUDED
#define X_PREAMBLE_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

/// Incomplete declaration to not raise warnings.
struct vertex_t;

// Prototypes that are shared across all versions.
/**
 * @brief This function tells whether the vertex \p v has a message to read.
 * @details This function just checks if the vertex \p v has a message to read
 * or not, it does not consume the message.
 * @param[in] v The vertex to process.
 * @retval true The vertex has at least one message unread.
 * @retval false The vertex has no message left to read.
 * @pre \p v points to an allocated memory area containing a vertex.
 **/
bool has_message(struct vertex_t* v);
/**
 * @brief This function consumers the next message in vertex \p v mailbox.
 * @details This function consumes the next message in vertex \p v mailbox and
 * stores it in \p message. Prior to consuming the message, this function checks
 * if the vertex \p v actually has at least one message to read. The result of
 * this check is what this functions returns, which allows for this function to
 * be used in a loop condition statement.
 * @param[inout] v The vertex to process.
 * @param[out] message The variable in which to store the message consumed.
 * @retval true The vertex \p v had a message to read; this message has been 
 * consumed and is stored in \p message.
 * @retval false The vertex \p had no message left to read. The message \p
 * message is left untouched.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @pre \p message points to a memory area already allocated for a message.
 **/
bool get_next_message(struct vertex_t* v, MESSAGE_TYPE* message);
/**
 * @brief This function builds a vertex \p v from its representation in file \p
 * f.
 * @details Since this function is implemented by the end-user, post-conditions
 * cannot be asserted by the author of my_pregel.
 * @param[in] f The file to read from.
 * @param[in] v The vertex in which to store the vertex built from \p f.
 * @pre f points to a file successfully open for reading.
 * @pre \p v points to an allocated memory area containing a vertex.
 **/
void deserialise_vertex(FILE* f, struct vertex_t* v);
/**
 * @brief This function writes in a file the serialised representation of a
 * vertex.
 * @details IMPORTANT: This function being defined by the user, the post
 * conditions that can be given at this stage are limited.
 * @param[out] f The file into which write.
 * @param[in] v The vertex to serialise.
 * @pre f points to a file already successfully open.
 * @pre f points to a file open in read mode or read-write mode.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post f is still open.
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
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post All neighbours of vertex \p v will have received the message \p message
 * before next superstep. Note that it may be combined during the process.
 **/
void broadcast(struct vertex_t* v, MESSAGE_TYPE message);
/**
 * @brief This function halts the vertex \p v.
 * @param[out] v The vertex to halt.
 * @pre \p v points to a memory area already allocated for a vertex.
 * @post The vertex \p v is inactive.
 **/
void vote_to_halt(struct vertex_t* v);
/**
 * @brief This function writes the serialised representation of all vertices
 * in the file \p f.
 * @param[out] f The file to dump into.
 * @pre f points to a file already successfully open.
 * @pre f points to a file open in write mode or read-write mode.
 **/
void dump(FILE* f);
/**
 * @brief This function executes a malloc and checks the memory area was
 * successfully allocated, otherwise exits the program.
 * @param[in] size_to_malloc The size to allocate, in bytes.
 * @return A pointer on the memory area allocated.
 * @post If the function call completes, the pointer returns is guaranteed to
 * point to a memory area successfully allocated and containing \p size_of_malloc
 * bytes.
 **/
void* safe_malloc(size_t size_to_malloc);
/**
 * @brief This function executes a realloc and checks that it succeeded, 
 * otherwise exits the program.
 * @param[in] ptr A pointer on the memory area to reallocate.
 * @param[in] size_to_realloc The size of reallocate.
 * @return A pointer on the memory area reallocated.
 * @pre \p ptr points to a memory area already allocated by a malloc or realloc.
 * @post If the function call completes, the pointer returns is guaranteed to
 * point to a memory area successfully reallocated and containing \p
 * size_of_realloc bytes.
 **/
void* safe_realloc(void* ptr, size_t size_to_realloc);

// User-defined functions
/**
 * @brief This function combines two messages into one.
 * @details This function must be implemented by the user and is available only
 * if the program is compiled with the use of combiners as indicated with the 
 * compilation flag -DUSE_COMBINER. Nonetheless, the post condition(s) must be
 * respected. 
 * @param[inout] message_a The reference message.
 * @param[in] message_b The message to compare.
 * @pre \p message_a points to an allocated memory area containing a message.
 * @pre \p message_b points to an allocated memory area containing a message.
 * @post \p message_a contains the combined value.
 **/
extern void combine(MESSAGE_TYPE* message_a, MESSAGE_TYPE* _message_b);
/**
 * @brief This function performs the actual superstep calculations of a vertex.
 * @details This function must be defined by the user.
 * @param[inout] v The vertex to process.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post The vertex \p v has finished his work for the current superstep.
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
