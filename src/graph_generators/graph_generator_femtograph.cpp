/**
 * @file graph_generator_femtograph.cpp
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
#include <random>
#include <sstream>
#include <vector>

std::default_random_engine generator;
unsigned int numberOfNodes;

/**
 * @brief This function returns the next ID available, taking in considerations the exclusions given.
 *
 * This algorithm has been taken from a reply on Stack Overflow, available at https://stackoverflow.com/questions/6443176/how-can-i-generate-a-random-number-within-a-range-but-exclude-some. The profile page of its author, Howard, is available at https://stackoverflow.com/users/577423/howard.
 **/
unsigned int nextID(const std::vector<unsigned int>& exclusions);

/**
 * @brief This function generates a graph containing X nodes.
 * @param[in] argc The number of arguments.
 * @param[in] argv The arguments.
 * @resval EXIT_SUCCESS Program execution successful.
 * @resval EXIT_FAILURE Failure in program execution.
 **/
int main(int argc, char* argv[])
{
	const unsigned int GRAPH_SIZE = 1;
	const unsigned int OUTPUT_FILE = 1 << 1;
	const unsigned int MAX_NUMBER_OF_EDGES_PER_NODE = 1 << 2;
	unsigned int argumentChecker = 0;
	std::string argValue;
	std::string argName;

	std::string outputFileName;
	unsigned int maxEdgeCountPerNode = 0;

	for(int i = 1; i < argc-1; i+=2)
	{
		argName = argv[i];
		argValue = argv[i+1];
    	
		if(argName == "--graphSize")
		{
        	numberOfNodes = std::stoi(argValue);
			argumentChecker |= GRAPH_SIZE;
		}
		else if(argName == "--output")
		{
			outputFileName = argValue;
			argumentChecker |= OUTPUT_FILE;
		}
		else if(argName == "--maxOutDegree")
		{
			maxEdgeCountPerNode = std::stoi(argValue);
			argumentChecker |= MAX_NUMBER_OF_EDGES_PER_NODE;
		}
		else
		{
			std::cerr << "Unknown argument: " << argName << std::endl;
			exit(-1);
		}
    }

	if(!(argumentChecker & OUTPUT_FILE))
	{
		std::cout << "Missing output file information." << std::endl;
		exit(-1);
	}
	else if(!(argumentChecker & GRAPH_SIZE))
	{
		std::cout << "Missing graph size information." << std::endl;
		exit(-1);
	}

	FILE* outputFile = fopen(outputFileName.c_str(), "w");
	if(outputFile == NULL)
	{
		std::cout << "Cannot open the output file: " << argv[2] << std::endl;
		exit(-1);
	}

	/////////////////////////////////
	// PROGRAM START
	/////////////////////////////////
	std::vector<unsigned int> exclusions;
	unsigned int numberOfEdges = 0;
	unsigned int iteratorEdges = 0;
	unsigned int destination = 0;
	unsigned int totalNumberOfEdges = 0;

	// Sets the seed of the random number generator to have reproductable results.
	int nodeSeed = 0;
	srand(nodeSeed);

	const int CHUNK_PERCENT = 10;
	int chunk = (numberOfNodes - (numberOfNodes % CHUNK_PERCENT)) / CHUNK_PERCENT;
	int progress = 0;

	for(unsigned int source = 0; source < numberOfNodes; source++)
	{
		if((argumentChecker & MAX_NUMBER_OF_EDGES_PER_NODE) != 0 && maxEdgeCountPerNode > 0)
		{
			numberOfEdges += 1 + (rand() % maxEdgeCountPerNode);
		}
		else
		{
			numberOfEdges += 1 + (rand() % (numberOfNodes - 2));
		}
	}
	srand(nodeSeed);

	numberOfEdges=0;

	for(unsigned int source = 0; source < numberOfNodes; source++)
	{
		exclusions.clear();
		exclusions.push_back(source);
		
		if((argumentChecker & MAX_NUMBER_OF_EDGES_PER_NODE) != 0 && maxEdgeCountPerNode > 0)
		{
			numberOfEdges = 1 + (rand() % maxEdgeCountPerNode);
		}
		else
		{
			numberOfEdges = 1 + (rand() % (numberOfNodes - 2));
		}
		
		totalNumberOfEdges += numberOfEdges;
#ifdef DEBUG_VERSION
		std::cout << "Vertex " << source << " will have " << numberOfEdges << " neighbours." << std::endl;
#endif
		iteratorEdges = 0;
		while(iteratorEdges < numberOfEdges)
		{
			destination = nextID(exclusions);
			exclusions.push_back(destination);
#ifdef DEBUG_VERSION
			std::cout << source << "->" <<  destination << std::endl;
#endif

			fprintf(outputFile, "%u %u\n", source, destination);
			iteratorEdges++;
		}

		if(source > 0 && source % chunk == 0)
		{
		 	progress += CHUNK_PERCENT;
			std::cout << progress << "% of the graph produced." << std::endl;
		}
	}

	std::cout << numberOfNodes << " nodes created, with a total of " << totalNumberOfEdges << " edges." << std::endl;

	return 0; 
}

unsigned int nextID(const std::vector<unsigned int>& exclusions)
{
	unsigned int rangeLength = (numberOfNodes-1) - exclusions.size();
	std::uniform_int_distribution<unsigned int> distribution(0, rangeLength);
	unsigned int randomInt = distribution(generator);

	for(unsigned int i = 0; i < exclusions.size(); i++)
	{
		if(exclusions[i] > randomInt)
		{
			return randomInt;
		}

		randomInt++;
	} 	

	return randomInt;
}
