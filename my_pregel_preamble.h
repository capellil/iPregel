/**
 * @file my_pregel_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef X_PREAMBLE_H_INCLUDED
#define X_PREAMBLE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#ifdef USE_COMBINER
	#include <pthread.h> // for pthread_mutex_t
#endif

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
void deserialiseVertex(FILE* f, struct vertex_t* v);
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
 * @brief This message halts the vertex \p v.
 * @param[out] v The vertex to halt.
 * @pre v points to a memory area already allocated for a vertex.
 * @post The vertex \p is inactive.
 **/
void vote_to_halt(struct vertex_t* v);
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
 * @param[in[ size_to_realloc The size of reallocate.
 * @return A pointer on the memory area reallocated.
 **/
void* safe_realloc(void* ptr, size_t size_to_realloc);
#ifndef USE_COMBINE
	/**
	 * @brief This function removes unread messages in vertex mailboxes.
	 * @param[in] id The identifier of the vertex to process.
 	 * @post The mailbox of the vertex identified by id is empty.
	 **/
	void reset_inbox(VERTEX_ID id);
#endif // if(n)def USE_COMBINER

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
	#ifdef USE_SPIN_LOCK
		#define VERTEX_STRUCTURE bool active; \
								 bool voted_to_halt; \
								 bool has_message; \
								 bool has_message_next; \
								 unsigned int neighbours_count; \
								 VERTEX_ID id; \
								 MESSAGE_TYPE message; \
								 MESSAGE_TYPE message_next; \
								 pthread_spinlock_t spinlock; \
								 VERTEX_ID* neighbours;
	#else
		#define VERTEX_STRUCTURE bool active; \
								 bool voted_to_halt; \
								 bool has_message; \
								 bool has_message_next; \
								 unsigned int neighbours_count; \
								 VERTEX_ID id; \
								 MESSAGE_TYPE message; \
								 MESSAGE_TYPE message_next; \
								 pthread_mutex_t mutex; \
								 VERTEX_ID* neighbours;
	#endif
#else // ifndef USE_COMBINER
	#define VERTEX_STRUCTURE bool active; \
							 bool voted_to_halt; \
							 VERTEX_ID id; \
							 unsigned int neighbours_count; \
							 VERTEX_ID* neighbours;
	/**
	 * @brief This structure acts as the mailbox of a vertex.
	 **/
	struct messagebox_t
	{
		/// This variable contains the maximum number of mails that this inbox can contain.
		size_t max_message_number;
		/// This variable contains the current number of mails in this inbox.
		size_t message_number;
		/// This variable contains the number of mails read in this inbox.
		size_t message_read;
		/// This variable contains the actual messages.
		MESSAGE_TYPE* messages;
	};
	/// This variable contains the inbox for each vertex.
	struct messagebox_t* all_inboxes[OMP_NUM_THREADS];
	/// This variable contains the inbox for the next superstep for each vertex.
	struct messagebox_t* all_inboxes_next_superstep[OMP_NUM_THREADS];
#endif // if(n)def USE_COMBINER

#endif // X_PREAMBLE_H_INCLUDED
