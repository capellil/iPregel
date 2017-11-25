#include <stdlib.h>
#include <string.h>

typedef unsigned int VERTEX_ID;
struct my_vector_t
{
	size_t size;
	VERTEX_ID* buff;
};

const unsigned int FLAG_MESSAGE = 0;
const unsigned int FLAG_MAILBOX = 1;
struct my_mailbox_t
{
	unsigned int type;
	union
	{
		VERTEX_ID message;
		struct my_vector_t mailbox;
	};
};

typedef struct my_mailbox_t MESSAGE_TYPE;
#include "my_pregel_preamble.h"
struct vertex_t
{
	VERTEX_STRUCTURE
	struct my_vector_t value;
};
#include "my_pregel_postamble.h"

void append_message_to_mailbox(struct my_vector_t* v, VERTEX_ID message)
{
	if(v->size == 0)
	{
		v->buff = (unsigned int*)safe_malloc(sizeof(unsigned int));
	}
	else
	{
		v->buff = (unsigned int*)safe_realloc(v->buff, sizeof(unsigned int) * (v->size + 1));	
	}
	
	v->buff[v->size] = message;
	v->size++;
}

void compute(struct vertex_t* v)
{
	if(superstep == 0)
	{
		struct my_mailbox_t m;
		m.type = FLAG_MESSAGE;
		m.message = v->id;
		broadcast(v, m);
		v->value.size = 0;
	}
	else if(superstep == 1)
	{
		MESSAGE_TYPE temp;
		if(get_next_message(v, &temp))
		{
			if(temp.type == FLAG_MESSAGE)
			{
				append_message_to_mailbox(&v->value, temp.message);
			}
			else
			{
				v->value = temp.mailbox;
			}
		}
	}
	vote_to_halt(v);
}

void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b)
{
	if(a->type == FLAG_MESSAGE)
	{
		VERTEX_ID temp = a->message;
		a->type = FLAG_MAILBOX;
		a->mailbox.size = 0;
		a->mailbox.buff = NULL;
		append_message_to_mailbox(&a->mailbox, temp);
	}

	append_message_to_mailbox(&a->mailbox, b->message);
}

void deserialise_vertex(FILE* f, struct vertex_t* v)
{
	size_t fread_size = fread(&v->id, sizeof(VERTEX_ID), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from deserialise vertex.\n");
		exit(-1);
	}
	fread_size = fread(&v->out_neighbours_count, sizeof(unsigned int), 1, f);
	if(fread_size != 1)
	{
		printf("Error in fread from deserialise vertex.\n");
		exit(-1);
	}
	if(v->out_neighbours_count > 0)
	{
		v->out_neighbours = (unsigned int*)safe_malloc(sizeof(VERTEX_ID) * v->out_neighbours_count);
		fread_size = fread(&v->out_neighbours[0], sizeof(VERTEX_ID), v->out_neighbours_count, f);
		if(fread_size != v->out_neighbours_count)
		{
			printf("Error in fread from deserialise vertex.\n");
			exit(-1);
		}
	}
}

void serialise_vertex(FILE* f, struct vertex_t* v)
{
	if(fwrite(&v->id, sizeof(VERTEX_ID), 1, f) != 1)
	{
		perror("Failed to write a vertex id in the output file:");
		exit(-1);
	}
	
	unsigned int size = v->value.size;
	if(fwrite(&size, sizeof(unsigned int), 1, f) != 1)
	{
		perror("Failed to write a vertex input out_neighbours count in the output file:");
		exit(-1);
	}
	if(size > 0)
	{
		if(fwrite(v->value.buff, sizeof(VERTEX_ID), size, f) != size)
		{
			perror("Failed to write a vertex input out_neighbours in the output file:");
			exit(-1);
		}
	}
}

int main(int argc, char* argv[])
{
	if(argc != 3) 
	{
		printf("Incorrect number of parameters.\n");
		return -1;
	}

	if(strcmp(argv[1], argv[2]) == 0)
	{
		printf("The input and output files should be different.\n");
		return -1;
	}

	FILE* f = fopen(argv[1], "rb");
	if(!f)
	{
		perror("File opening failed.");
		return -1;
	}

	unsigned int number_of_vertices = 0;
	if(fread(&number_of_vertices, sizeof(unsigned int), 1, f) != 1)
	{
		perror("Could not read the number of vertices.");
		return -1;
	}
	init(f, number_of_vertices);
	fclose(f);
	run();

	f = fopen(argv[2], "wb");
	if(!f)
	{
		perror("Failed to open the output file:");
		return -1;
	}

	if(fwrite(&number_of_vertices, sizeof(unsigned int), 1, f) != 1)
	{
		perror("Failed to write the number of vertices in the output file:");
		return -1;
	}
	
	dump(f);
	fclose(f);

	return EXIT_SUCCESS;
}
