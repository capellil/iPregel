#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <limits>
#include <cstdint>

std::default_random_engine generator;
size_t numberOfVertices;
size_t totalNumberOfEdges;
size_t maxOutDegree;
const unsigned int GRAPH_SIZE = 1;
const unsigned int OUTPUT_FILE = 1 << 1;
const unsigned int MAX_OUT_DEGREE = 1 << 2;

/**
 * @brief This function returns the next ID available, taking in considerations the exclusions given.
 *
 * This algorithm has been taken from https://stackoverflow.com/questions/6443176/how-can-i-generate-a-random-number-within-a-range-but-exclude-some.
 **/
template <typename VERTEX_ID>
VERTEX_ID nextID(const std::vector<VERTEX_ID>& exclusions)
{
	VERTEX_ID rangeLength = (numberOfVertices-1) - exclusions.size();
	std::uniform_int_distribution<VERTEX_ID> distribution(0, rangeLength);
	VERTEX_ID randomInt = distribution(generator);

	for(VERTEX_ID i = 0; i < exclusions.size(); i++)
	{
		if(exclusions[i] > randomInt)
		{
			return randomInt;
		}

		randomInt++;
	} 	

	return randomInt;
}

template <typename VERTEX_ID>
void startGenerating(FILE* configFile, FILE* idxFile, FILE* adjFile)
{
	/////////////////////////////////
	// PROGRAM START
	/////////////////////////////////
	std::vector<VERTEX_ID> exclusions;
	size_t numberOfEdges = 0;
	VERTEX_ID iteratorEdges = 0;
	VERTEX_ID destination = 0;

	// Sets the seed of the random number generator to have reproductable results.
	int chunk = 1000000;
	srand(0);

	for(VERTEX_ID source = 0; source < numberOfVertices; source++)
	{
		exclusions.clear();
		exclusions.push_back(source);
		
		fwrite(&totalNumberOfEdges, sizeof(size_t), 1, idxFile);
		
		numberOfEdges = 1 + (rand() % maxOutDegree);
		totalNumberOfEdges += numberOfEdges;

		iteratorEdges = 0;
		while(iteratorEdges < numberOfEdges)
		{
			destination = nextID(exclusions);
			exclusions.push_back(destination);
			fwrite(&destination, sizeof(VERTEX_ID), 1, adjFile);
			iteratorEdges++;
		}

		if(source % chunk == 0)
		{
			std::cout << source / chunk << "M vertices created." << std::endl;
		}
	}

	fprintf(configFile, "%zu %zu %zu", numberOfVertices, totalNumberOfEdges, maxOutDegree);

	std::cout << numberOfVertices << " nodes created, with a total of " << totalNumberOfEdges << " edges." << std::endl;
}

/**
 * @brief This function generates a graph containing X nodes.
 * 
 * The graph is output in binary format as follows:
 * <ul>
 *     <li>4B int: source ID</li>
 *     <li>4B int: destination ID</li>
 * <ul>
 *     <li>argv[0]: Name of the executable file.</li>
 *     <li>argv[1]: Number of nodes to generate.</li> 
 *     <li>argv[2]: Output file.</li> 
 * </ul>
 * @param[in] argc The number of arguments.
 * @param[in] argv The arguments.
 * @resval EXIT_SUCCESS Program execution successful.
 * @resval EXIT_FAILURE Failure in program execution.
 **/
int main(int argc, char* argv[])
{
	std::string argValue;
	std::string argName;
	std::string outputFileName;
	unsigned int argumentChecker = 0;
	bool binaryOutput = false;
	bool edgeList = true;

	for(int i = 1; i < argc-1; i+=2)
	{
		argName = argv[i];
		argValue = argv[i+1];
    	
		if(argName == "--output")
		{
			outputFileName = argValue;
			argumentChecker |= OUTPUT_FILE;
		}
		else if(argName == "--graphSize")
		{
			std::stringstream converter;
			converter << argValue;
			converter >> numberOfVertices;
			argumentChecker |= GRAPH_SIZE;
		}
		else if(argName == "--maxOutDegree")
		{
			maxOutDegree = std::stoi(argValue);
			argumentChecker |= MAX_OUT_DEGREE;
		}
		else
		{
			std::cerr << "Unknown argument: " << argName << std::endl;
			exit(-1);
		}
    }

	bool atLeastOneMissingInfo = false;
	if(!(argumentChecker & OUTPUT_FILE))
	{
		std::cerr << "Missing output file information, try again and pass me \"--output <path to the output file>\"." << std::endl;
		atLeastOneMissingInfo = true;
	}

	if(!(argumentChecker & GRAPH_SIZE))
	{
		std::cerr << "Missing graph size information, try again and pass me \"--graphSize <number of vertices>\"." << std::endl;
		atLeastOneMissingInfo = true;
	}

	if(!(argumentChecker & MAX_OUT_DEGREE))
	{
		std::cerr << "Missing max degree information, try again and pass me \"--maxOutDegree <max number of out degree for any one vertex>\"." << std::endl;
		atLeastOneMissingInfo = true;
	}

	if(atLeastOneMissingInfo)
	{
		exit(-1);
	}

	// Config file
	std::string configFileName = outputFileName;
	configFileName += ".config";
	FILE* configFile = fopen(configFileName.c_str(), "w");
	if(configFile == NULL)
	{
		std::cerr << "Cannot open the output file: \"" << configFileName << "\"." << std::endl;
		exit(-1);
	}

	// Idx file
	std::string idxFileName = outputFileName;
	idxFileName += ".idx";
	FILE* idxFile = fopen(idxFileName.c_str(), "wb");
	if(idxFile == NULL)
	{
		std::cerr << "Cannot open the output file: \"" << idxFileName << "\"." << std::endl;
		exit(-1);
	}

	// Adj file
	std::string adjFileName = outputFileName;
	adjFileName += ".adj";
	FILE* adjFile = fopen(adjFileName.c_str(), "wb");
	if(adjFile == NULL)
	{
		std::cerr << "Cannot open the output file: \"" << adjFileName << "\"." << std::endl;
		exit(-1);
	}

	std::cout << "Outputing config to \"" << configFileName << "\", idx to \"" << idxFileName << "\" and adj to \"" << adjFileName << "\"." << std::endl;
	std::cout << "Creating a graph with " << numberOfVertices << " vertices." << std::endl;
	std::cout << "Vertices will have at most " << maxOutDegree << " out neighbours." << std::endl;

	startGenerating<unsigned int>(configFile, idxFile, adjFile);

	fclose(configFile);
	fclose(idxFile);
	fclose(adjFile);
	return 0; 
}
