#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

struct vertex_t
{
	std::vector<unsigned int> out_neighbours;
	std::vector<unsigned int> in_neighbours;
};

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Incorrect number of arguments." << std::endl;
		return -1;
	}
	
	std::ifstream f;
	f.open(argv[1]);
	
	std::string line;
	unsigned int edges_count;
	unsigned int vertices_count;
	unsigned int vertex_id;
	unsigned int neighbour_id;
	std::stringstream ss;
	
	std::getline(f, line);
	f >> line;
	f >> edges_count;
	std::cout << edges_count << " edges" << std::endl;
	f >> vertices_count;
	std::cout << vertices_count << " vertices" << std::endl;

	struct vertex_t* all_vertices = new struct vertex_t[vertices_count+1]; // because it is 1-indexed
	
	// Read the end of the line
	std::getline(f, line);

	unsigned int max = 0;
	for(unsigned int i = 0; i < 172183984; i++)
	{
		f >> vertex_id;
		if(vertex_id > max) max = vertex_id;
		f >> neighbour_id;
		if(neighbour_id > max) max = neighbour_id;
		if(vertex_id > vertices_count)
		{
			std::cout << "Overflow: " << vertex_id << std::endl;
			return -1;
		}
		all_vertices[vertex_id].out_neighbours.push_back(neighbour_id);
		all_vertices[neighbour_id].in_neighbours.push_back(vertex_id);
	}

	std::cout << "Input file entirely processed." << std::endl;
	
	FILE* output = fopen(argv[2], "wb");
	
	if(output == NULL)
	{
		std::cerr << "Failure in opening the output file." << std::endl;
		return -1;
	}
	fwrite(&vertices_count, sizeof(unsigned int), 1, output);

	unsigned int temp;
	for(unsigned int i = 1; i <= vertices_count; i++)
	{
		fwrite(&i, sizeof(unsigned int), 1, output);
		temp = all_vertices[i].out_neighbours.size();
		fwrite(&temp, sizeof(unsigned int), 1, output);
		if(temp > 0)
		{
			fwrite(all_vertices[i].out_neighbours.data(), sizeof(unsigned int), temp, output);
		}
		temp = all_vertices[i].in_neighbours.size();
		fwrite(&temp, sizeof(unsigned int), 1, output);
		if(temp > 0)
		{
			fwrite(all_vertices[i].in_neighbours.data(), sizeof(unsigned int), temp, output);
		}
	}
	
	return 0;
}

