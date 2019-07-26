/**
 * @file contiguouer.cpp
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
#include <unordered_map>
#include <cstdlib>

unsigned int nextAvailableId = 0;
std::unordered_map<size_t, size_t> idHashmap;

unsigned int getContiguousId(unsigned int arbitraryId)
{
	unsigned int contiguousId;
	if(idHashmap.find(arbitraryId) == idHashmap.end())
	{
		idHashmap[arbitraryId] = nextAvailableId;
		contiguousId = nextAvailableId;
		nextAvailableId++;
	}
	else
	{
		contiguousId = idHashmap[arbitraryId];
	}

	return contiguousId;
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Incorrect number of arguments, this program should be invoked like : \"" << argv[0] << " <filePath>\"." << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream graph;
	graph.open(argv[1], std::ios_base::in | std::ios_base::binary);
	if(!graph.is_open())
	{
		throw std::runtime_error("Cannot open the graph input file.");
	}

	std::ofstream contiguousGraph;
	std::string contiguousGraphPath(argv[1]);
	contiguousGraphPath += "OUTPUT";
	contiguousGraph.open(contiguousGraphPath, std::ios_base::out | std::ios_base::binary);
	if(!contiguousGraph.is_open())
	{
		throw std::runtime_error("Cannot open the graph output file: \"" + contiguousGraphPath + "\".");
	}

	unsigned int numberOfVertices;
	graph.read((char*)&numberOfVertices, sizeof(unsigned int));
	contiguousGraph.write((char*)&numberOfVertices, sizeof(unsigned int));
	std::cout << "Number of vertices: " << numberOfVertices << std::endl;
	
	unsigned int vertexIdAndNumberOfNeighbours[2];
	unsigned int* neighbours;
	for(unsigned int i = 0; i < numberOfVertices; i++)
	{
		// Source vertex id + number of neighbours
		graph.read((char*)vertexIdAndNumberOfNeighbours, sizeof(unsigned int) * 2);
		vertexIdAndNumberOfNeighbours[0] = getContiguousId(vertexIdAndNumberOfNeighbours[0]);
		contiguousGraph.write((char*)vertexIdAndNumberOfNeighbours, sizeof(unsigned int) * 2);

		neighbours = (unsigned int*)malloc(sizeof(unsigned int) * vertexIdAndNumberOfNeighbours[1]);
		graph.read((char*)neighbours, sizeof(unsigned int) * vertexIdAndNumberOfNeighbours[1]);
		
		// Out neighbours
		for(unsigned int j = 0; j < vertexIdAndNumberOfNeighbours[1]; j++)
		{
			neighbours[j] = getContiguousId(neighbours[j]);
		}

		contiguousGraph.write((char*)neighbours, sizeof(unsigned int) * vertexIdAndNumberOfNeighbours[1]);
		free(neighbours);
	}

	graph.close();
	contiguousGraph.close();

	return EXIT_SUCCESS;
}
