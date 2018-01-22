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
	ss = std::istringstream();
	ss.str(argv[3]);
	VERTEX_ID_TYPE edgeCount;
	ss >> edgeCount;
	contiguousGraph << vertexCount << " " << edgeCount << std::endl;
	VERTEX_ID_TYPE edgeCountReal = 0;
	ss = std::istringstream();

	while(std::getline(graph, line) && line.size() > 0 && (line[0] == '#' || line[0] == '%'))
	{
		// Skip header
	}

	try
	{
		do
		{
			ss.str(line);
			ss >> src;
			src = getContiguousId(src);
			ss >> dest;
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
