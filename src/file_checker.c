#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	if(argc == 1)
	{
		printf("You must pass the path to the file to check.\n");
		return EXIT_FAILURE;
	}

	FILE* f = fopen(argv[1], "rb");
	if(!f)
	{
		perror("Cannot open the file:");
		return EXIT_FAILURE;
	}

	unsigned int number_of_vertices;
	if(fread(&number_of_vertices, sizeof(unsigned int), 1, f) != 1)
	{
		printf("Failure in reading the number of vertices.\n");
		return EXIT_FAILURE;
	}

	printf("There are %u vertices in the graph according to the file header.\n", number_of_vertices);

	unsigned int vertex_id = 0;
	unsigned int number_of_neighbours = 0;
	unsigned int max_number_of_neighbours = 1;
	unsigned int* neighbours = malloc(sizeof(unsigned int));
	for(unsigned int i = 0; i < number_of_vertices; i++)
	{
		printf("Read vertex id: ");
		fflush(stdout);
		if(fread(&vertex_id, sizeof(unsigned int), 1, f) != 1)
		{
			printf("failed.\n");
			return EXIT_FAILURE;
		}
		printf("%u\n", vertex_id);

		printf("Reading the number of neighbours of that vertex: ");
		fflush(stdout);
		if(fread(&number_of_neighbours, sizeof(unsigned int), 1, f) != 1)
		{
			printf("failed.\n");
			return EXIT_FAILURE;
		}
		printf("%u\n", number_of_neighbours);

		if(number_of_neighbours > max_number_of_neighbours)
		{
			printf("Resizing the buffer from %u to %u neighbours.\n", max_number_of_neighbours, number_of_neighbours);
			neighbours = realloc(neighbours, sizeof(unsigned int) * number_of_neighbours);
			max_number_of_neighbours = number_of_neighbours;
		}

		if(fread(neighbours, sizeof(unsigned int), number_of_neighbours, f) != number_of_neighbours)
		{
			printf("Failure in reading the vertex id\n");
			return EXIT_FAILURE;
		}

		for(unsigned int i = 0; i < number_of_neighbours; i++)
		{
			printf("\t%u -> %u\n", vertex_id, neighbours[i]);
		}
	}

	printf("File completely read: ");
	fflush(stdout);
	if(feof(f))
	{
		printf("yes.\n");
	}
	else
	{
		size_t error_state;
		char temp;
		while(!feof(f))
		{
			error_state = fread(&temp, sizeof(char), 1, f);

			if(error_state == 0)
			{
				printf("yes.\n");
				break;
			}
			else if(error_state != 1)
			{
				perror("Failure in reading the additional information in the file");
				return EXIT_FAILURE;
			}

			printf("'%c'", temp);
		}
	}

	fclose(f);

	return EXIT_SUCCESS;
}
