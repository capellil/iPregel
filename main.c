#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#ifdef USE_COMBINER
#include <pthread.h>
#endif

#define VERTEX_ID unsigned int
#define MESSAGE_TYPE unsigned int

struct vertex_t
{
	bool active;
	VERTEX_ID id;
	MESSAGE_TYPE value;
	unsigned int neighbours_count;
	VERTEX_ID* neighbours;
#ifdef USE_COMBINER
	bool has_message;
	MESSAGE_TYPE message;
	bool has_message_next;
	MESSAGE_TYPE message_next;
	pthread_mutex_t mutex;
#endif
};

#ifdef USE_COMBINER
#else
struct messagebox_t
{
	size_t max_message_number;
	size_t message_number;
	size_t message_read;
	MESSAGE_TYPE* messages;
};
#endif

unsigned int active_vertices = 0;
unsigned int messages_left = 0;
unsigned int messages_left_omp[OMP_NUM_THREADS] = {0};
unsigned int superstep = 0;
struct vertex_t* all_vertices = NULL;

#ifdef USE_COMBINER
#else
struct messagebox_t* all_inboxes[OMP_NUM_THREADS];
struct messagebox_t* all_inboxes_next_superstep[OMP_NUM_THREADS];
#endif

bool has_message(struct vertex_t* v);
bool get_next_message(struct vertex_t* v, MESSAGE_TYPE* message_value);
void deserialiseVertex(FILE* f, struct vertex_t* v);
void serialiseVertex(FILE* f);
void send_message(VERTEX_ID id, MESSAGE_TYPE message);
#ifdef USE_COMBINER
#else
void reset_inbox(VERTEX_ID id);
#endif
void vote_to_halt(struct vertex_t* v);
void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b);
void* safe_malloc(size_t size_to_malloc);
void* safe_realloc(void* ptr, size_t size_to_realloc);

void compute(struct vertex_t* v)
{
	if(superstep == 0)
	{
		v->value = v->id;

		for(unsigned int i = 0; i < v->neighbours_count; i++)
		{
			send_message(v->neighbours[i], v->value);
		}

		vote_to_halt(v);
	}
	else
	{
		MESSAGE_TYPE valueTemp = v->value;
		MESSAGE_TYPE message_value;
		while(get_next_message(v, &message_value))
		{
			if(v->value > message_value)
			{
				v->value = message_value;
			}
		}

		if(valueTemp != v->value)
		{
			for(unsigned int i = 0; i < v->neighbours_count; i++)
			{
				send_message(v->neighbours[i], v->value);
			}
		}

		vote_to_halt(v);
	}
}

void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b)
{
	if(*a > *b)
	{
		*a = *b;
	}
} 

bool has_message(struct vertex_t* v)
{
#ifdef USE_COMBINER
	return v->has_message;
#else
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		if(all_inboxes[i][v->id].message_read < all_inboxes[i][v->id].message_number)
		{
			return true;
		}
	}
	return false;
#endif
}

bool get_next_message(struct vertex_t* v, MESSAGE_TYPE* message_value)
{
#ifdef USE_COMBINER
	if(v->has_message)
	{
		*message_value = v->message;
		v->has_message = false;
		messages_left_omp[omp_get_thread_num()]--;
		return true;
	}
	return false;
#else
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		if(all_inboxes[i][v->id].message_read < all_inboxes[i][v->id].message_number)
		{
			*message_value = all_inboxes[i][v->id].messages[all_inboxes[i][v->id].message_read];
			all_inboxes[i][v->id].message_read++;
			messages_left_omp[omp_get_thread_num()]--;
			return true;
		}
	}

	return false;
#endif
}


void deserialiseVertex(FILE* f, struct vertex_t* v)
{
	v->active = true;
	size_t fread_size = fread(&v->id, sizeof(VERTEX_ID), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from deserialise vertex.\n");
		exit(-1);
	}
	fread_size = fread(&v->neighbours_count, sizeof(unsigned int), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from deserialise vertex.\n");
		exit(-1);
	}
	v->neighbours = (unsigned int*)safe_malloc(sizeof(VERTEX_ID) * v->neighbours_count);
	if(v->neighbours > 0 )
	{
		fread_size = fread(&v->neighbours[0], sizeof(VERTEX_ID), v->neighbours_count, f);
		if(fread_size != v->neighbours_count)
		{
			printf("Error in fread from deserialise vertex.\n");
			exit(-1);
		}
	}
}

void serialiseVertex(FILE* f)
{

}

void send_message(VERTEX_ID id, MESSAGE_TYPE message)
{
#ifdef USE_COMBINER
	pthread_mutex_lock(&all_vertices[id].mutex);
	
	if(all_vertices[id].has_message_next)
	{
		combine(&all_vertices[id].message_next, &message);
		pthread_mutex_unlock(&all_vertices[id].mutex);
	}
	else
	{
		all_vertices[id].has_message_next = true;
		all_vertices[id].message_next = message;
		pthread_mutex_unlock(&all_vertices[id].mutex);
		messages_left_omp[omp_get_thread_num()]++;
	}
#else
	all_inboxes_next_superstep[omp_get_thread_num()][id].message_number++;
	if(all_inboxes_next_superstep[omp_get_thread_num()][id].message_number > all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number)
	{
		all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number = all_inboxes_next_superstep[omp_get_thread_num()][id].message_number;
		all_inboxes_next_superstep[omp_get_thread_num()][id].messages = safe_realloc(all_inboxes_next_superstep[omp_get_thread_num()][id].messages, sizeof(MESSAGE_TYPE) * all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number);
	}
	
	all_inboxes_next_superstep[omp_get_thread_num()][id].messages[all_inboxes_next_superstep[omp_get_thread_num()][id].message_number-1] = message;
	messages_left_omp[omp_get_thread_num()]++;
#endif
}

#ifdef USE_COMBINER
#else
void reset_inbox(VERTEX_ID id)
{
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		#pragma omp atomic
		messages_left -= all_inboxes[i][id].message_number;
		all_inboxes[i][id].message_number = 0;
		all_inboxes[i][id].message_read = 0;
	}
}
#endif

void vote_to_halt(struct vertex_t* v)
{
	v->active = false;
}

void* safe_malloc(size_t size_to_malloc)
{
	void* ptr = malloc(size_to_malloc);
	if(ptr == NULL)
	{
		exit(-1);
	}
}

void* safe_realloc(void* ptr, size_t size_to_realloc)
{
	ptr = realloc(ptr, size_to_realloc);
	if(ptr == NULL)
	{
		exit(-1);
	}
	return ptr;
}

int main(int argc, char* argv[])
{
	if(argc != 2) 
	{
		printf("Incorrect number of parameters.\n");
		return -1;
	}

	FILE* f = fopen(argv[1], "rb");
	if(!f)
	{
		perror("File opening failed.");
		return -1;
	}

	unsigned int vertices_count = 0;
	size_t fread_size = fread(&vertices_count, sizeof(unsigned int), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from main.\n");
		exit(-1);
	}
	all_vertices = (struct vertex_t*)safe_malloc(sizeof(struct vertex_t) * vertices_count);
	
	// Allocate the inbox for each thread
#ifdef USE_COMBINER
#else
	#pragma omp parallel for default(none) shared(all_inboxes, vertices_count, all_inboxes_next_superstep)
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		all_inboxes[i] = (struct messagebox_t*)safe_malloc(sizeof(struct messagebox_t) * vertices_count);
		all_inboxes_next_superstep[i] = (struct messagebox_t*)safe_malloc(sizeof(struct messagebox_t) * vertices_count);
	}
#endif

	// Deserialise all the vertices
	for(unsigned int i = 0; i < vertices_count && !feof(f); i++)
	{
		deserialiseVertex(f, &all_vertices[i]);
		active_vertices++;
	}

	// Allocate the inbox for each vertex in each thread's inbox.
#ifdef USE_COMBINER
	for(unsigned int i = 0; i < vertices_count; i++)
	{
		for(unsigned int j = 0; j < omp_get_num_threads(); j++)
		{
			all_vertices[i].has_message = false;
			all_vertices[i].has_message_next = false;
			pthread_mutex_init(&all_vertices[i].mutex, NULL);
		}
	}

#else
	#pragma omp parallel for default(none) shared(vertices_count, all_inboxes, all_inboxes_next_superstep, all_vertices)
	for(unsigned int j = 0; j < omp_get_num_threads(); j++)
	{
		for(unsigned int i = 0; i < vertices_count; i++)
		{
			all_inboxes[j][i].max_message_number = 1;
			all_inboxes[j][i].messages = (MESSAGE_TYPE*)safe_malloc(all_inboxes[j][i].max_message_number);
			all_inboxes[j][i].message_number = 0;
			all_inboxes[j][i].message_read = 0;
			all_inboxes_next_superstep[j][i].max_message_number = 1;
			all_inboxes_next_superstep[j][i].messages = (MESSAGE_TYPE*)safe_malloc(all_inboxes_next_superstep[j][i].max_message_number);
			all_inboxes_next_superstep[j][i].message_number = 0;
			all_inboxes_next_superstep[j][i].message_read = 0;
		}
	}
#endif
	printf("Active vertices = %u\n", active_vertices);

	double timer_superstep_start = 0;
	double timer_superstep_stop = 0;
	while(active_vertices != 0 || messages_left > 0)
	{
		timer_superstep_start = omp_get_wtime();
		active_vertices = 0;
#ifdef USE_COMBINER
		#pragma omp parallel default(none) shared(vertices_count, all_vertices, active_vertices, messages_left, messages_left_omp)
#else
		#pragma omp parallel default(none) shared(all_inboxes, all_inboxes_next_superstep, vertices_count, all_vertices, active_vertices, messages_left, messages_left_omp)
#endif
		{
			#pragma omp for reduction(+:active_vertices)
			for(unsigned int i = 0; i < vertices_count; i++)
			{
				if(all_vertices[i].active || has_message(&all_vertices[i]))
				{
					all_vertices[i].active = true;
					active_vertices++;
					compute(&all_vertices[i]);
#ifndef USE_COMBINER
					reset_inbox(i);
#endif
				}
			}

			#pragma omp for reduction(+:messages_left)
			for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
			{
				messages_left += messages_left_omp[i];
				messages_left_omp[i] = 0;
			}
			
#ifdef USE_COMBINER
			// Combine all messages to let only one message to each vertex for next superstep.
			#pragma omp for reduction(-:active_vertices)
			for(unsigned int i = 0; i < vertices_count; i++)
			{
				if(all_vertices[i].active)
				{
					active_vertices--;
				}

				if(all_vertices[i].has_message_next)
				{
					all_vertices[i].has_message = true;
					all_vertices[i].message = all_vertices[i].message_next;
					all_vertices[i].has_message_next = false;
				}
			}
#else
			#pragma omp for reduction(-:active_vertices)
			for(unsigned int i = 0; i < vertices_count; i++)
			{
				if(all_vertices[i].active)
				{
					active_vertices--;
				}
			}
#endif
		}

#ifdef USE_COMBINER
#else
		for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
		{
			struct messagebox_t* inbox_swap = all_inboxes[i];
			all_inboxes[i] = all_inboxes_next_superstep[i];
			all_inboxes_next_superstep[i] = inbox_swap;
		}
#endif
		timer_superstep_stop = omp_get_wtime();
		printf("Superstep %u finished in %fs; %u active vertices and %u messages left.\n", superstep, timer_superstep_stop - timer_superstep_start, active_vertices, messages_left);
		superstep++;
	}

	printf("Verification...");
	for(unsigned int i = 0; i < vertices_count; i++)
	{
		if(all_vertices[i].value != 0)
		{
			printf("failed.\n");
			exit(-1);
		}
	}
	printf("succeeded.\n");

	return EXIT_SUCCESS;
}
