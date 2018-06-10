#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

struct vertex_t
{
	std::vector<unsigned int> out_neighbours;
	std::vector<unsigned int> in_neighbours;
};

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Incorrect number of arguments, please invoke this program"
			      << " like: " << argv[0] << " <graph_to_convert> <output_file>"
				  << std::endl;
		return -1;
	}
	
	std::ifstream f;
	f.open(argv[1]);
	if(!f.is_open())
	{
		std::string inputFilePath("Cannot open the graph input file \"");
		inputFilePath += argv[1];
		inputFilePath += "\".";
		throw std::runtime_error(inputFilePath);
	}
	
	unsigned int edges_count;
	unsigned int vertices_count;
	unsigned int vertex_id;
	unsigned int neighbour_id;
	
	f >> vertices_count;
	std::cout << vertices_count << " vertices" << std::endl;
	f >> edges_count;
	std::cout << edges_count << " edges" << std::endl;

	struct vertex_t* all_vertices = new struct vertex_t[vertices_count];

	unsigned int edges_count_real = 0;	
	for(unsigned int i = 0; i < edges_count; i++)
	{
		f >> vertex_id;
		f >> neighbour_id;
		if(vertex_id > vertices_count)
		{
			std::cout << "Overflow: " << vertex_id << std::endl;
			return -1;
		}
		all_vertices[vertex_id - IP_ID_OFFSET].out_neighbours.push_back(neighbour_id);
		all_vertices[neighbour_id].in_neighbours.push_back(vertex_id - IP_ID_OFFSET);
		edges_count_real++;
	}

	std::cout << "Real number of edges = " << edges_count_real << ", expected " << edges_count << std::endl;

	std::cout << "Input file read, all vertices created with their neighbours." << std::endl;
	
	FILE* output = fopen(argv[2], "w");
	
	if(output == NULL)
	{
		std::cerr << "Failure in opening the output file." << std::endl;
		return -1;
	}
	fprintf(output, "AdjacencyGraph\n");
	fprintf(output, "%u\n", vertices_count);
	fprintf(output, "%u\n", edges_count);

	unsigned int temp = 0;
	for(unsigned int i = 0; i < vertices_count; i++)
	{
		temp += all_vertices[i].out_neighbours.size();
		fprintf(output, "%u\n", temp);
	}
	
	for(unsigned int i = 0; i < vertices_count; i++)
	{
		for(unsigned int j = 0; j < all_vertices[i].out_neighbours.size(); j++)
		{
			fprintf(output, "%u\n", all_vertices[i].out_neighbours[j]); // Because ligra is 0-indexed, unlike the SNAP graphs.
		}
	}

	return 0;
}

