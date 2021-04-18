/**
 * @file iPregel_preamble.h
 * @copyright Copyright (C) 2019 Ludovic Capelli
 * @par License
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * @author Ludovic Capelli
 * @brief This file acts as a manager of all the versions available.
 * @details The program iPregel contains several versions, each optimised for
 * a given set of assuiptions. This file is in charge of selecting the right
 * version according to the assuiptions passed via compiler flags.
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

#if defined(IP_NEEDS_OUT_NEIGHBOUR_IDS) && !defined(IP_NEEDS_OUT_NEIGHBOUR_COUNT)
	#define IP_NEEDS_OUT_NEIGHBOUR_COUNT
#endif // #if defined(IP_NEEDS_OUT_NEIGHBOUR_IDS) && !defined(IP_NEEDS_OUT_NEIGHBOUR_COUNT)

#if defined(IP_NEEDS_IN_NEIGHBOUR_IDS) && !defined(IP_NEEDS_IN_NEIGHBOUR_COUNT)
	#define IP_NEEDS_IN_NEIGHBOUR_COUNT
#endif // #if defined(IP_NEEDS_IN_NEIGHBOUR_IDS) && !defined(IP_NEEDS_IN_NEIGHBOUR_COUNT)

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

/// This variable contains the current superstep number. It is 0-indexed.
size_t ip_superstep = 0;
/// This variable contains the total number of edges.
size_t ip_edges_count = 0;
/// This variable contains the total number of vertices.
size_t ip_vertices_count = 0;
/// This variable contains the number of active vertices at an instant t.
size_t ip_active_vertices = 0;
/// Forward declaration of the vertex structure to not raise warnings
struct ip_vertex_t;
/// This variable contains all the vertices.
struct ip_vertex_t* ip_all_vertices = NULL;
/// The number of threads available for processing.
int ip_thread_count;

// Functions to access global variables.
/**
 * @brief This function returns the current superstep.
 * @details The superstep in 0 indexed, that is, the first superstep is the
 * superstep 0.
 * @return The current superstep.
 **/
size_t ip_get_superstep();
/**
 * @brief This function tells if the current superstep is the first one.
 * @retval true The current superstep is the first one.
 * @retval false The current superstep is not the first one.
 **/
bool ip_is_first_superstep();
/**
 * @brief This function increments the current superstep index.
 **/
void ip_increment_superstep();
/**
 * @brief This function sets the number of vertices to \p vertices_count.
 * @param[in] vertices_count The number of vertices.
 **/
void ip_set_vertices_count(size_t vertices_count);
/**
 * @brief This function returns the total number of vertices.
 * @return The total number of vertices.
 **/
size_t ip_get_vertices_count();
/**
 * @brief This function sets the number of edges to \p edges_count.
 * @param[in] vertices_count The number of edges.
 **/
void ip_set_edges_count(size_t edges_count);
/**
 * @brief This function returns the total number of edges.
 * @return The total number of edges.
 **/
size_t ip_get_edges_count();
/**
 * @brief This function returns the vertex located at index \p location in the
 * global structure containing all vertices.
 * @param[in] location The location of the vertex in the global structure.
 * @return The vertex residing at location \p location in the global vertex
 * structure.
 **/
struct ip_vertex_t* ip_get_vertex_by_location(size_t location);
/**
 * @brief This function returns the vertex identified by \p id.
 * @param[in] id The identifier of the vertex to fetch.
 * @return The vertex identified by \p id.
 **/
struct ip_vertex_t* ip_get_vertex_by_id(IP_VERTEX_ID_TYPE id);

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
bool ip_has_message(struct ip_vertex_t* v);
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
bool ip_get_next_message(struct ip_vertex_t* v, IP_MESSAGE_TYPE* message);
/**
 * @brief This function sends the message \p message to the vertex identified 
 * by \p id.
 * @param[in] id The identifier of the destination vertex.
 * @param[in] message The message to send.
 * @pre id is an existing vertex identifier.
 * @post The message is delivered to the destination vertex. If a combiner is
 * used, this message may be ignored.
 **/
void ip_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE message);
/**
 * @brief This function sends the message \p message to all neighbours of the
 * vertex \p v.
 * @param[out] v The vertex broadcasting.
 * @param[in] message The message to broadcast.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post All neighbours of vertex \p v will have received the message \p message
 * before next superstep. Note that it may be combined during the process.
 **/
void ip_broadcast(struct ip_vertex_t* v, IP_MESSAGE_TYPE message);
/**
 * @brief This function halts the vertex \p v.
 * @param[out] v The vertex to halt.
 * @pre \p v points to a memory area already allocated for a vertex.
 * @post The vertex \p v is inactive.
 **/
void ip_vote_to_halt(struct ip_vertex_t* v);
/**
 * @brief This function is called by the underlying implementation version to initialise each vertex attribute according to the vertex structure used by that implementation version.
 * @param[in] first The ID of the first vertex to initialise.
 * @param[in] last The ID of the last vertex to initialise.
 * @pre first <= last
 **/
extern void ip_init_vertex_range(IP_VERTEX_ID_TYPE first, IP_VERTEX_ID_TYPE last);
/**
 * @brief This function loads the graph whose root name is \p file_path.
 * @param[in] file_path The root name of the graph (".config" / ".adj" / ".idx" suffixes will be added to it).
 * @param[in] directed Indicates whether the graph to load contains directed or undirected edges.
 * @param[in] weighted Indicates whether the graph to load contains weighted or unweighted edges.
 **/
void ip_load_graph(const char* file_path, bool directed, bool weighted);

/******************
 * SAFE FUNCTIONS *
 ******************/
/**
 * @brief This function executes a malloc and checks the memory area was
 * successfully allocated, otherwise exits the program.
 * @param[in] size_to_malloc The size to allocate, in bytes.
 * @return A pointer on the memory area allocated.
 * @post If the function call completes, the pointer returns is guaranteed to
 * point to a memory area successfully allocated and containing \p size_of_malloc
 * bytes.
 **/
void* ip_safe_malloc(size_t size_to_malloc);
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
void* ip_safe_realloc(void* ptr, size_t size_to_realloc);
/**
 * @brief This function frees the memory allocated by a pointer.
 * @details In case the pointer is NULL, nothing is done. It avoids double-free
 * problems. It also sets the pointer to NULL once the free is finished.
 * @pre Either \p ptr is a valid non-NULL pointer, either it is a NULL pointer.
 * @post ptr == NULL
 **/
void ip_safe_free(void* ptr);
/**
 * @brief This function opens a file and exits on failure.
 * @param[in] file_path The path leading to the file.
 * @param[in] mode The opening mode.
 * @return A pointer on the opened file.
 * @post Returned value != NULL
 **/
FILE* ip_safe_fopen(const char* file_path, const char* mode);
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
void ip_safe_fread(void * ptr, size_t size, size_t count, FILE * stream);
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
void ip_safe_fwrite(void * ptr, size_t size, size_t count, FILE * stream);

/**************************
 * USER-DEFINED FUNCTIONS *
 **************************/
/**
 * @brief This function combines two messages into one.
 * @param[inout] message_a The existing message in the vertex mailbox.
 * @param[in] message_b The message that just arrived for reception.
 * @pre \p message_a points to an allocated memory area containing a message.
 * @pre The operation defining the combination is associative and commutative.
 * @post \p message_a contains the combined value.
 **/
extern void ip_combine(IP_MESSAGE_TYPE* message_a, IP_MESSAGE_TYPE message_b);
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
extern void ip_serialise_vertex(FILE* f, struct ip_vertex_t* v);
/**
 * @brief This function performs the actual superstep calculations of a vertex.
 * @details This function must be defined by the user.
 * @param[inout] v The vertex to process.
 * @pre \p v points to an allocated memory area containing a vertex.
 * @post The vertex \p v has finished his work for the current superstep.
 **/
extern void ip_compute(struct ip_vertex_t* v);

/****************************
 * FUNCTIONS TO RUN IPREGEL *
 ****************************/
/**
 * @brief This function initialises the environment and architecture of 
 * iPregel.
 * @param[in] file_path Path leading to the file containing the graph. If a graph myGraph contains the files myGraph.config, myGraph.idx and myGraph.adj, it is "myGraph" that should be passed to this function.
 * @param[in] number_of_threads The number of threads to use.
 * @param[in] schedule The OpenMP schedule to apply in graphd loading and vertex processing. This helps ensuring that vertices are processed on the same thread they were loaded.
 * @param[in] chunk_size The chunk size to assign to the OpenMP schedule passed. Can be 0 to represent the default chunk size.
 * @param[in] directed Indicates whether the graph to load contains directed or undirected edges.
 * @param[in] weighted Indicates whether the graph to load contains weighted or unweighted edges.
 **/
void ip_init(const char* file_path, int number_of_threads, const char* schedule, int chunk_size, bool directed, bool weighted);
/**
 * @brief This function is implemented by underlying iPregel version to do their own initialisation.
 * @details This function is distinct from the global initialisation ip_init().
 **/
extern void ip_init_specific();
/**
 * @brief This function acts as the start point of the iPregel simulation.
 * @return The error code.
 * @retval 0 Success.
 **/
extern int ip_run();
/**
 * @brief This function writes the serialised representation of all vertices
 * in the file \p f.
 * @param[out] f The file to dump into.
 * @pre f points to a file already successfully open.
 * @pre f points to a file open in write mode or read-write mode.
 **/
void ip_dump(FILE* f);
	
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
