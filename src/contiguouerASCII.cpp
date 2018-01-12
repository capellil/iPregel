#include <iostream>
#include <fstream>
#include <sstream>
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
	size_t src;
	size_t dest;
	size_t vertexCount;
	size_t edgeCount = 0;

	while(std::getline(graph, line) && line.size() > 0 && line[0] == '#')
	{
		// Skip header
	}

	do
	{
		ss.str(line);
		ss >> src;
		src = getContiguousId(src);
		ss >> dest;
		dest = getContiguousId(dest);
		contiguousGraph << src << " " << dest << std::endl;
		edgeCount++;
	} while(std::getline(graph, line));
	vertexCount = src < dest ? dest : src;

	std::cout << "The " << vertexCount + 1 << " vertices now range from 0 to " << vertexCount << ", and " << edgeCount << " edges." << std::endl;

	graph.close();
	contiguousGraph.close();

	return EXIT_SUCCESS;
}
