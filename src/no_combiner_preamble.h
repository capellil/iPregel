/**
 * @file no_combiner_preamble.h
 * @author Ludovic Capelli
 **/

#ifndef NO_COMBINER_PREAMBLE_H_INCLUDED
#define NO_COMBINER_PREAMBLE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

// Global variables
/// This variable contains the number of active vertices at an instant t.
size_t mp_active_vertices = 0;
/// This variable contains the number of messages that have not been read yet.
size_t mp_messages_left = 0;
/// This variable is used for multithreading reduction into message_left.
size_t mp_messages_left_read_omp[OMP_NUM_THREADS] = {0};
/// This variable is used for multithreading reduction into message_left.
size_t mp_messages_left_sent_omp[OMP_NUM_THREADS] = {0};
/**
 * @brief This structure acts as the mailbox of a vertex.
 **/
struct mp_messagebox_t
{
	/// This variable contains the maximum number of mails that this inbox can contain.
	size_t max_message_number;
	/// This variable contains the current number of mails in this inbox.
	size_t message_number;
	/// This variable contains the number of mails read in this inbox.
	size_t message_read;
	/// This variable contains the actual messages.
	MP_MESSAGE_TYPE* messages;
};
/// This variable contains the inbox for each vertex.
struct mp_messagebox_t* mp_all_inboxes[OMP_NUM_THREADS];
/// This variable contains the inbox for the next superstep for each vertex.
struct mp_messagebox_t* mp_all_inboxes_next_superstep[OMP_NUM_THREADS];

// Prototypes
/**
 * @brief This function adds the message \p message to the vertex mailbox \p m.
 * @param[out] m The mailbox to use.
 * @param[in] message The message to add.
 * @pre \p m points to an allocated memory area containing a mailbox.
 * @post The message \p message is added to the mailbox \p m.
 **/
void mp_append_message_to_mailbox(struct mp_messagebox_t* m, MP_MESSAGE_TYPE message);
/**
 * @brief This function removes unread messages in vertex mailboxes.
 * @param[in] id The identifier of the vertex to process.
 * @post The mailbox of the vertex identified by id is empty.
 **/
void mp_reset_inbox(MP_VERTEX_ID_TYPE id);

/// This macro defines the minimal attributes of a vertex.
#define MP_VERTEX_STRUCTURE MP_VERTEX_ID_TYPE* out_neighbours; \
						 	MP_VERTEX_ID_TYPE* in_neighbours; \
	 		 			 	MP_NEIGHBOURS_COUNT_TYPE out_neighbours_count; \
						 	MP_NEIGHBOURS_COUNT_TYPE in_neighbours_count; \
						 	bool active; \
						 	MP_VERTEX_ID_TYPE id;

#endif // NO_COMBINER_PREAMBLE_H_INCLUDED
