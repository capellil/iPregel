/**
 * @file contiguouerASCII.cpp
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
#include <sstream>
#include <unordered_map>
#include <cstdlib>

typedef unsigned int VERTEX_ID_TYPE;

VERTEX_ID_TYPE nextAvailableId = 0;
std::unordered_map<VERTEX_ID_TYPE, VERTEX_ID_TYPE> idHashmap;

VERTEX_ID_TYPE getContiguousId(VERTEX_ID_TYPE arbitraryId)
{
	VERTEX_ID_TYPE contiguousId;
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
	if(argc != 4)
	{
		std::cerr << "Incorrect number of arguments, this program should be invoked like : \"" << argv[0] << " <filePath> <#vertices> <#edges>\"." << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream graph;
	graph.open(argv[1], std::ios_base::in);
	if(!graph.is_open())
	{
		throw std::runtime_error("Cannot open the graph input file.");
	}

	std::ofstream contiguousGraph;
	std::string contiguousGraphPath(argv[1]);
	contiguousGraphPath += "OUTPUT";
	contiguousGraph.open(contiguousGraphPath, std::ios_base::out);
	if(!contiguousGraph.is_open())
	{
		throw std::runtime_error("Cannot open the graph output file: \"" + contiguousGraphPath + "\".");
	}

	std::string line;
	std::istringstream ss;
	VERTEX_ID_TYPE src;
	VERTEX_ID_TYPE dest;
	ss.str(argv[2]);
	VERTEX_ID_TYPE vertexCount;
	ss >> vertexCount;
	std::istringstream ss2;
	ss2.str(argv[3]);
	VERTEX_ID_TYPE edgeCount;
	ss2 >> edgeCount;
	contiguousGraph << vertexCount << " " << edgeCount << std::endl;
	VERTEX_ID_TYPE edgeCountReal = 0;
	std::istringstream ss3;

	while(std::getline(graph, line) && line.size() > 0 && (line[0] == '#' || line[0] == '%'))
	{
		// Skip header
	}

	try
	{
		do
		{
			ss3.str(line);
			ss3 >> src;
			src = getContiguousId(src);
			ss3 >> dest;
			dest = getContiguousId(dest);
			contiguousGraph << src << " " << dest << std::endl;
			edgeCountReal++;
		} while(std::getline(graph, line));
	}
	catch(std::bad_alloc& e)
	{
		std::cout << "Exception raised, it seems there is not enough RAM to process this graph." << std::endl;
	}
	std::cout << "|V| given = " << vertexCount << ", |V| observed = " << nextAvailableId << std::endl;
	std::cout << "|E| given = " << edgeCount << ", |E| observed = " << edgeCountReal << std::endl;
	std::cout << "The vertex identifiers now range from 0 to " << nextAvailableId - 1 << "." << std::endl;

	graph.close();
	contiguousGraph.close();

	return EXIT_SUCCESS;
}
