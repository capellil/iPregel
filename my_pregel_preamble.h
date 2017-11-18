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
unsigned int active_vertices = 0;
unsigned int messages_left = 0;
unsigned int messages_left_omp[OMP_NUM_THREADS] = {0};
unsigned int superstep = 0;
unsigned int vertices_count = 0;
struct vertex_t* all_vertices = NULL;

// Prototypes
bool has_message(struct vertex_t* v);
bool get_next_message(struct vertex_t* v, MESSAGE_TYPE* message_value);
void deserialiseVertex(FILE* f, struct vertex_t* v);
void serialiseVertex(FILE* f);
void send_message(VERTEX_ID id, MESSAGE_TYPE message);
void vote_to_halt(struct vertex_t* v);
void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b);
void* safe_malloc(size_t size_to_malloc);
void* safe_realloc(void* ptr, size_t size_to_realloc);
#ifndef USE_COMBINER
	void reset_inbox(VERTEX_ID id);
#endif // if(n)def USE_COMBINER

// User-defined functions
extern void compute(struct vertex_t* v);
extern void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b);
extern int verify();

// Functions defined in x_postamble.h
extern int init(FILE* f, unsigned int number_of_vertices);
extern int run();

#ifdef USE_COMBINER
	#define VERTEX_STRUCTURE bool active; \
							 bool voted_to_halt; \
							 VERTEX_ID id; \
							 unsigned int neighbours_count; \
							 VERTEX_ID* neighbours; \
							 bool has_message; \
							 MESSAGE_TYPE message; \
							 bool has_message_next; \
							 MESSAGE_TYPE message_next; \
							 pthread_mutex_t mutex;
#else // ifndef USE_COMBINER
	#define VERTEX_STRUCTURE bool active; \
							 bool voted_to_halt; \
							 VERTEX_ID id; \
							 unsigned int neighbours_count; \
							 VERTEX_ID* neighbours; 
	struct messagebox_t
	{
		size_t max_message_number;
		size_t message_number;
		size_t message_read;
		MESSAGE_TYPE* messages;
	};
	struct messagebox_t* all_inboxes[OMP_NUM_THREADS];
	struct messagebox_t* all_inboxes_next_superstep[OMP_NUM_THREADS];
#endif // if(n)def USE_COMBINER

#endif // X_PREAMBLE_H_INCLUDED
