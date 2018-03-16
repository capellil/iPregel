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
 * This file must be included AFTER the user defined the types IP_VERTEX_ID_TYPE_TYPE
 * and IP_MESSAGE_TYPE.
 **/

#ifndef MY_PREGEL_PREAMBLE_H_INCLUDED
#define MY_PREGEL_PREAMBLE_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

/// This variable contains the current superstep number. It is 0-indexed.
size_t mp_superstep = 0;
/// This variable contains the current meta superstep number. It is 0-indexed.
size_t mp_meta_superstep = 0;
/// This variable contains the number of meta supersteps to execute.
size_t mp_meta_superstep_count = 1;
/// This variable contains the total number of vertices.
size_t mp_vertices_count = 0;
/// Incomplete declaration to not raise warnings.
struct mp_vertex_t;
/// This variable contains all the vertices.
struct mp_vertex_t* mp_all_vertices = NULL;

// Functions to access global variables.
/**
 * @brief This function returns the current superstep.
 * @details The superstep in 0 indexed, that is, the first superstep is the
 * superstep 0.
 * @return The current superstep.
 **/
size_t mp_get_superstep();
/**
 * @brief This function tells if the current superstep is the first one.
 * @retval true The current superstep is the first one.
 * @retval false The current superstep is not the first one.
 **/
bool mp_is_first_superstep();
/**
 * @brief This function increments the current superstep index.
 **/
void mp_increment_superstep();
/**
 * @brief This function resets the superstep index.
 * @details Supersteps are 0-indexed, therefore this function resets the index
 * of supersteps to 0.
 **/
void mp_reset_superstep();
/**
 * @brief This function returns the current meta superstep.
 * @details The meta superstep is 0 indexed, that is, the first meta superstep
 * is the meta superstep 0.
 * @return The current meta superstep.
 **/
size_t mp_get_meta_superstep();
/**
 * @brief This function tells if the current meta superstep is the first one.
 * @retval true The current meta superstep is the first one.
 * @retval false The current meta superstep is not the first one.
 **/
bool mp_is_first_meta_superstep();
/**
 * @brief This function increments the current meta superstep index.
 **/
void mp_increment_meta_superstep();
/**
 * @brief This function returns the number of meta supersteps to run.
 * @return The number of meta supersteps to run.
 **/
size_t mp_get_meta_superstep_count();
/**
 * @brief This function sets the number of meta supersteps to run.
 * @details By default, the number of meta supersteps to run is 1.
 * @param[in] meta_superstep_count The number of meta supersteps to run.
 * @pre \p meta_superstep_count >= 1
 **/
void mp_set_meta_superstep_count(size_t meta_superstep_count);
/**
 * @brief This function sets the number of vertices to \p vertices_count.
 * @param[in] vertices_count The number of vertices.
 **/
void mp_set_vertices_count(size_t vertices_count);
/**
 * @brief This function retursn the total number of vertices.
 * @return The total number of vertices.
 **/
size_t mp_get_vertices_count();
/**
 * @brief This function returns the vertex located at index \p location in the
 * global structure containing all vertices.
 * @param[in] location The location of the vertex in the global structure.
 * @return The vertex residing at location \p location in the global vertex
 * structure.
 **/
struct mp_vertex_t* mp_get_vertex_by_location(size_t location);
/**
 * @brief This function returns the vertex identified by \p id.
 * @param[in] id The identifier of the vertex to fetch.
 * @return The vertex identified by \p id.
 **/
struct mp_vertex_t* mp_get_vertex_by_id(IP_VERTEX_ID_TYPE id);

// Functions for the user
/**
 * @brief This function tells whether the vertex \p v has a message to read.
 * @details This function just checks if the vertex \p v has a message to read
 * or not, it does not consume the message.
 * @param[in] v The vertex to process.
 * @retval true The vertex has at least one message unread.
 * @retval false The vertex has no message left to read.
 * @pre \p v points to an allocated memory area containing a vertex.
 **/
bool mp_has_message(struct mp_vertex_t* v);
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
bool mp_get_next_message(struct mp_vertex_t* v, IP_MESSAGE_TYPE* message);
/**
 * @brief This function sends the message \p message to the vertex identified 
 * by \p id.
 * @param[in] id The identifier of the destination vertex.
 * @param[in] message The message to send.
 * @pre id is an existing vertex identifier.
 * @post The message is delivered to the destination vertex. If a combiner is
 * used, this message may be ignored.
 **/
void mp_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE message);
/**
 * @brief This function sends the message \p message to all neighbours of the
 * vertex \p v.
 * @param[out] v The vertex broadcasting.
 * @param[in] message The message to broadcast.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post All neighbours of vertex \p v will have received the message \p message
 * before next superstep. Note that it may be combined during the process.
 **/
void mp_broadcast(struct mp_vertex_t* v, IP_MESSAGE_TYPE message);
/**
 * @brief This function halts the vertex \p v.
 * @param[out] v The vertex to halt.
 * @pre \p v points to a memory area already allocated for a vertex.
 * @post The vertex \p v is inactive.
 **/
void mp_vote_to_halt(struct mp_vertex_t* v);
/**
 * @brief This function adds a new edge from \p src to \p dest.
 * @param[in] src The source vertex identifier.
 * @param[in] dest The destination vertex identifier.
 **/
void mp_add_edge(IP_VERTEX_ID_TYPE src, IP_VERTEX_ID_TYPE dest);
/**
 * @brief This function writes the serialised representation of all vertices
 * in the file \p f.
 * @param[out] f The file to dump into.
 * @pre f points to a file already successfully open.
 * @pre f points to a file open in write mode or read-write mode.
 **/
void mp_dump(FILE* f);
/**
 * @brief This function executes a malloc and checks the memory area was
 * successfully allocated, otherwise exits the program.
 * @param[in] size_to_malloc The size to allocate, in bytes.
 * @return A pointer on the memory area allocated.
 * @post If the function call completes, the pointer returns is guaranteed to
 * point to a memory area successfully allocated and containing \p size_of_malloc
 * bytes.
 **/
void* mp_safe_malloc(size_t size_to_malloc);
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
void* mp_safe_realloc(void* ptr, size_t size_to_realloc);
/**
 * @brief This function frees the memory allocated by a pointer.
 * @details In case the pointer is NULL, nothing is done. It avoids double-free
 * problems. It also sets the pointer to NULL once the free is finished.
 * @pre Either \p ptr is a valid non-NULL pointer, either it is a NULL pointer.
 * @post ptr == NULL
 **/
void mp_safe_free(void* ptr);
/**
 * @brief This function reads from a file and checks that it succeeded,
 * otherwise exits the program.
 * @param[out] ptr A pointer on the buffer to fill.
 * @param[in] size The size of an element.
 * @param[in] count The number of elements to read.
 * @param[inout] stream A pointer on the source file to write to.
 * @pre \p stream points to a file already opened in read-only or read-write mode.
 * @pre \p ptr points to a memory area already allocated and containing at least
 * \p size * \p count bytes.
 * @pre \p size >= 1
 * @pre \p count >= 1
 **/
void mp_safe_fread(void * ptr, size_t size, size_t count, FILE * stream);
/**
 * @brief This function writes to a file a checks that it succeeded, otherwise
 * exits the program.
 * @param[in] ptr A pointer on the buffer to write.
 * @param[in] size The size of an element.
 * @param[in] count The number of elements to write.
 * @param[inout] stream A pointer on the source file to write to.
 * @pre \p stream points to a file already opened in write-only or read-write mode.
 * @pre \p ptr points to a memory area already allocated and containing at least
 * \p size * \p count bytes.
 * @pre \p size >= 1
 **/
void mp_safe_fwrite(void * ptr, size_t size, size_t count, FILE * stream);

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
 * @post \p message_a contains the combined value.
 **/
extern void mp_combine(IP_MESSAGE_TYPE* message_a, IP_MESSAGE_TYPE message_b);
/**
 * @brief This function is user-defined, and is in charge of loading the vertices.
 * @details There is no constraint about the graph source, this function is solely
 * expected to have deserialised the entire graph once it completes.
 * @post The graph is entirely deserialised.
 **/
extern void mp_deserialise();
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
extern void mp_serialise_vertex(FILE* f, struct mp_vertex_t* v);
/**
 * @brief This function performs the actual superstep calculations of a vertex.
 * @details This function must be defined by the user.
 * @param[inout] v The vertex to process.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post The vertex \p v has finished his work for the current superstep.
 **/
extern void mp_compute(struct mp_vertex_t* v);
/**
 * @brief This function initialises the environment and architecture of 
 * my_pregel.
 * @param[inout] f The file from which deserialising vertices.
 * @param[in] number_of_vertices The number of vertices to load from the file.
 * @param[in] number_of_edges The number of edges to load from the file.
 * @retval 0 Success.
 **/
extern int mp_init(FILE* f, size_t number_of_vertices, size_t number_of_edges);
/**
 * @brief This function acts as the start point of the my_pregel simulation.
 * @return The error code.
 * @retval 0 Success.
 **/
extern int mp_run();

#ifdef IP_USE_SPREAD
	#ifdef IP_USE_SINGLE_BROADCAST
		#include "combiner_spread_single_broadcast_preamble.h"
	#else // ifndef IP_USE_SINGLE_BROADCAST
		#include "combiner_spread_preamble.h"
	#endif // if(n)def IP_USE_SINGLE_BROADCAST
#else // ifndef IP_USE_SPREAD
	#ifdef IP_USE_SINGLE_BROADCAST
		#include "combiner_single_broadcast_preamble.h"
	#else // ifndef IP_USE_SINGLE_BROADCAST
		#include "combiner_preamble.h"
	#endif // if(n)def IP_USE_SINGLE_BROADCAST
#endif // if(n)def IP_USE_SPREAD
	
#endif // MY_PREGEL_PREAMBLE_H_INCLUDED
