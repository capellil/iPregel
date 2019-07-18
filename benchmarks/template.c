// 1) User-defined types
typedef unsigned int IP_VERTEX_ID_TYPE;
typedef IP_VERTEX_ID_TYPE IP_NEIGHBOUR_COUNT_TYPE;
typedef IP_VERTEX_ID_TYPE IP_MESSAGE_TYPE;
typedef IP_VERTEX_ID_TYPE IP_VALUE_TYPE;

// 2) Include iPregel header
#include "iPregel.h"

// 3) User-defined functions
void ip_compute(struct ip_vertex_t* v)
{
	// Write the calculation to do on every vertex
}

void ip_combine(IP_MESSAGE_TYPE* old, IP_MESSAGE_TYPE new)
{
	// Write the combination to apply
	// Ex: to keep minimum: if(*old > new) *old = new;
	// Ex: to keep maximum: if(*old < new) *old = new;
	// Ex: to get sum: *old += new;
	// Ex: to get product: *old *= new;
}

void ip_serialise_vertex(FILE* f, struct ip_vertex_t* v)
{
	// Write the information you want about that vertex to the file
	// Ex: fwrite(&v->id, sizeof(IP_VERTEX_ID_TYPE), 1, f);
	// Ex: fwrite(&v->value, sizeof(IP_MESSAGE_TYPE), 1, f);
}

// 4) Write the main function to launch the application
int main(int argc, char* argv[])
{
	// Variables we are going to need for initialisation / dumping
	char input_graph_path[] = "/path/to/input/graph";
	char output_graph_path[] = "/path/to/output/graph";
	int number_of_threads = <your_number>;
	bool edges_are_directed = <true / false>;
	bool edges_are_weighted = <true / false>;

	// Initialisation
	ip_init(input_graph_path, number_of_threads, edges_are_directed, edges_are_weighted);

	// Run
	ip_run();

	// Dumping
	FILE* f_out = fopen(output_graph_path, "w");
	ip_dump(f_out);

	return 0;
}
