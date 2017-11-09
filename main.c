#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

unsigned int active_vertices = 0;
unsigned int message_left = 0;
unsigned int superstep = 0;
struct messagebox_t* all_inboxes[OMP_NUM_THREADS];
struct messagebox_t* all_inboxes_next_superstep[OMP_NUM_THREADS];

struct vertex_t
{
	bool active;
	unsigned int id;
	unsigned int value;
	unsigned int neighbours_count;
	unsigned int* neighbours;
};

struct messagebox_t
{
	size_t max_message_number;
	size_t message_number;
	size_t message_read;
	unsigned int* messages;
};

bool has_message(struct vertex_t* v);
bool get_next_message(struct vertex_t* v, unsigned int* message_value);
void deserialiseVertex(FILE* f, struct vertex_t* v);
void serialiseVertex(FILE* f);
void send_message(unsigned int id, unsigned int message);
void reset_inbox(unsigned int id);
void vote_to_halt(struct vertex_t* v);

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
		unsigned int valueTemp = v->value;
		unsigned int message_value;
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

bool has_message(struct vertex_t* v)
{
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		if(all_inboxes[i][v->id].message_read < all_inboxes[i][v->id].message_number)
		{
			return true;
		}
	}
	return false;
}

bool get_next_message(struct vertex_t* v, unsigned int* message_value)
{
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		if(all_inboxes[i][v->id].message_read < all_inboxes[i][v->id].message_number)
		{
			*message_value = all_inboxes[i][v->id].messages[all_inboxes[i][v->id].message_read];
			all_inboxes[i][v->id].message_read++;
			return true;
		}
	}

	return false;
}


void deserialiseVertex(FILE* f, struct vertex_t* v)
{
	v->active = true;
	fread(&v->id, sizeof(unsigned int), 1, f);
	fread(&v->neighbours_count, sizeof(unsigned int), 1, f);
	v->neighbours = (unsigned int*)malloc(sizeof(unsigned int) * v->neighbours_count);
	if(v->neighbours > 0 )
	{
		fread(&v->neighbours[0], sizeof(unsigned int), v->neighbours_count, f);
	}
}

void serialiseVertex(FILE* f)
{

}

void send_message(unsigned int id, unsigned int message)
{

	all_inboxes_next_superstep[omp_get_thread_num()][id].message_number++;
	if(all_inboxes_next_superstep[omp_get_thread_num()][id].message_number > all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number)
	{
		all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number = all_inboxes_next_superstep[omp_get_thread_num()][id].message_number;
		all_inboxes_next_superstep[omp_get_thread_num()][id].messages = realloc(all_inboxes_next_superstep[omp_get_thread_num()][id].messages, sizeof(unsigned int) * all_inboxes_next_superstep[omp_get_thread_num()][id].max_message_number);
	}
	
	all_inboxes_next_superstep[omp_get_thread_num()][id].messages[all_inboxes_next_superstep[omp_get_thread_num()][id].message_number-1] = message;
	#pragma omp atomic
	message_left++;
}

void reset_inbox(unsigned int id)
{
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		#pragma omp atomic
		message_left -= all_inboxes[i][id].message_number;
		all_inboxes[i][id].message_number = 0;
		all_inboxes[i][id].message_read = 0;
	}
}

void vote_to_halt(struct vertex_t* v)
{
	v->active = false;
	#pragma omp atomic
	active_vertices--;
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
	fread(&vertices_count, sizeof(unsigned int), 1, f);
	struct vertex_t* all_vertices = (struct vertex_t*)malloc(sizeof(struct vertex_t) * vertices_count);
	
	// Allocate the inbox for each thread
	#pragma omp parallel for default(none) shared(all_inboxes, vertices_count, all_inboxes_next_superstep)
	for(unsigned int i = 0; i < omp_get_num_threads(); i++)
	{
		all_inboxes[i] = (struct messagebox_t*)malloc(sizeof(struct messagebox_t) * vertices_count);
		all_inboxes_next_superstep[i] = (struct messagebox_t*)malloc(sizeof(struct messagebox_t) * vertices_count);
	}

	// Deserialise all the vertices
	for(unsigned int i = 0; i < vertices_count && !feof(f); i++)
	{
		deserialiseVertex(f, &all_vertices[i]);
		active_vertices++;
	}

	// Allocate the inbox for each vertex in each thread's inbox.
	#pragma omp parallel for default(none) shared(vertices_count, all_inboxes, all_inboxes_next_superstep)
	for(unsigned int j = 0; j < omp_get_num_threads(); j++)
	{
		for(unsigned int i = 0; i < vertices_count; i++)
		{
			all_inboxes[j][i].max_message_number = 1;
			all_inboxes[j][i].messages = (unsigned int*)malloc(all_inboxes[j][i].max_message_number);
			all_inboxes[j][i].message_number = 0;
			all_inboxes[j][i].message_read = 0;
			all_inboxes_next_superstep[j][i].max_message_number = 1;
			all_inboxes_next_superstep[j][i].messages = (unsigned int*)malloc(all_inboxes_next_superstep[j][i].max_message_number);
			all_inboxes_next_superstep[j][i].message_number = 0;
			all_inboxes_next_superstep[j][i].message_read = 0;
		}
	}
	printf("Active vertices = %u\n", active_vertices);

	while(active_vertices != 0 || message_left > 0)
	{
		active_vertices = 0;
		#pragma omp parallel default(none) shared(all_inboxes, all_inboxes_next_superstep, vertices_count, all_vertices, active_vertices)
		{
			#pragma omp for
			for(unsigned int i = 0; i < vertices_count; i++)
			{
				if(all_vertices[i].active || has_message(&all_vertices[i]))
				{
					all_vertices[i].active = true;
					#pragma omp atomic
					active_vertices++;
					compute(&all_vertices[i]);
					reset_inbox(i);
				}
			}
		}

		for(unsigned int i = 0; i < OMP_NUM_THREADS; i++)
		{
			struct messagebox_t* inbox_swap = all_inboxes[i];
			all_inboxes[i] = all_inboxes_next_superstep[i];
			all_inboxes_next_superstep[i] = inbox_swap;
		}
		printf("Superstep %u finished; %u active vertices and %u messages left.\n", superstep, active_vertices, message_left);
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