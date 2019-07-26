/**
 * @file graph_converter.cpp
 * @copyright Copyright (C) 2019 Ludovic Capelli
 * @par License
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * @author Ludovic Capelli
 **/
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
	
	unsigned int max = 0;
	for(unsigned int i = 0; i < edges_count; i++)
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

	std::cout << "Input file read, all vertices created with their neighbours." << std::endl;
	
	FILE* output = fopen(argv[2], "wb");
	
	if(output == NULL)
	{
		std::cerr << "Failure in opening the output file." << std::endl;
		return -1;
	}
	fwrite(&vertices_count, sizeof(unsigned int), 1, output);

	unsigned int temp;
	for(unsigned int i = 0; i < vertices_count; i++)
	{
		fwrite(&i, sizeof(unsigned int), 1, output);
		std::sort(all_vertices[i].out_neighbours.begin(), all_vertices[i].out_neighbours.end());
		all_vertices[i].out_neighbours.erase(std::unique(all_vertices[i].out_neighbours.begin(), all_vertices[i].out_neighbours.end()), all_vertices[i].out_neighbours.end());
		temp = all_vertices[i].out_neighbours.size();
		fwrite(&temp, sizeof(unsigned int), 1, output);
		if(temp > 0)
		{
			fwrite(all_vertices[i].out_neighbours.data(), sizeof(unsigned int), temp, output);
		}
		std::sort(all_vertices[i].in_neighbours.begin(), all_vertices[i].in_neighbours.end());
		all_vertices[i].in_neighbours.erase(std::unique(all_vertices[i].in_neighbours.begin(), all_vertices[i].in_neighbours.end()), all_vertices[i].in_neighbours.end());
		temp = all_vertices[i].in_neighbours.size();
		fwrite(&temp, sizeof(unsigned int), 1, output);
		if(temp > 0)
		{
			fwrite(all_vertices[i].in_neighbours.data(), sizeof(unsigned int), temp, output);
		}
	}
	
	return 0;
}

