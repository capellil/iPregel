#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <string> // std::stoi

std::default_random_engine generator;
std::normal_distribution<double> normalDistribution;
std::uniform_int_distribution<unsigned int> uniformDistribution;
unsigned int numberOfNodes;

/**
 * @brief This function returns the next ID available, taking in considerations the exclusions given.
 *
 * This algorithm has been taken from https://stackoverflow.com/questions/6443176/how-can-i-generate-a-random-number-within-a-range-but-exclude-some.
 **/
unsigned int nextID(const std::vector<unsigned int>& exclusions);

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
	const unsigned int GRAPH_SIZE = 1;
	const unsigned int OUTPUT_FILE = 1 << 1;
	const unsigned int MAX_EDGE_COUNT_PER_NODE = 1 << 2;
	const unsigned int MIN_EDGE_COUNT_PER_NODE = 1 << 3;
	const unsigned int NORMAL_DISTRIBUTION = 1 << 4;
	const unsigned int NORMAL_DISTRIBUTION_MEAN = 1 << 5;
	const unsigned int NORMAL_DISTRIBUTION_STDDEV = 1 << 6;
	const unsigned int BINARY_OUTPUT = 1 << 7;
	unsigned int argumentChecker = 0;
	std::string argValue;
	std::string argName;
	long long unsigned int fileSize = 0;

	std::string outputFileName;
	unsigned int maxEdgeCountPerNode = 0;
	unsigned int minEdgeCountPerNode = 0;
	double normalDistributionMean = 0.0;
	double normalDistributionStddev = 0.0;

	for(int i = 1; i < argc-1; i+=2)
	{
		argName = argv[i];
		argValue = argv[i+1];
    	
		if(argName == "-graphSize")
		{
        	numberOfNodes = std::stoi(argValue);
			argumentChecker |= GRAPH_SIZE;
		}
		else if(argName == "-o")
		{
			outputFileName = argValue;
			argumentChecker |= OUTPUT_FILE;
		}
		else if(argName == "-maxEdgeCountPerNode")
		{
			maxEdgeCountPerNode = std::stoul(argValue);
			argumentChecker |= MAX_EDGE_COUNT_PER_NODE;
		}
		else if(argName == "-minEdgeCountPerNode")
		{
			minEdgeCountPerNode = std::stoul(argValue);
			argumentChecker |= MIN_EDGE_COUNT_PER_NODE;
		}
		else if(argName == "-distribution")
		{
			if(argValue == "normal")
			{
				argumentChecker |= NORMAL_DISTRIBUTION;
			}
			else
			{
				std::cerr << "Unknown distribution: " << argValue << std::endl;
			}
		}
		else if(argName == "-normalDistributionMean")
		{
			normalDistributionMean = std::stoul(argValue);
			argumentChecker |= NORMAL_DISTRIBUTION_MEAN;
		}
		else if(argName == "-normalDistributionStddev")
		{
			normalDistributionStddev = std::stoul(argValue);
			argumentChecker |= NORMAL_DISTRIBUTION_STDDEV;
		}
		else if(argName == "-binaryOutput")
		{
			if(argValue == "yes")
			{
				argumentChecker |= BINARY_OUTPUT;
			}
			else
			{
				std::cerr << "Unknown binaryOutput information." << std::endl;
			}
		}
		else
		{
			std::cerr << "Unknown argument: " << argName << std::endl;
			exit(-1);
		}
    }

	if(!(argumentChecker & OUTPUT_FILE))
	{
		std::cout << "Missing output file information. Use \"-o <your_file_path>\"." << std::endl;
		exit(-1);
	}
	else if(!(argumentChecker & GRAPH_SIZE))
	{
		std::cout << "Missing graph size information. Use \"-graphSize <your_value>\"." << std::endl;
		exit(-1);
	}

	if(!(argumentChecker & MIN_EDGE_COUNT_PER_NODE))
	{
		minEdgeCountPerNode = 1;
	}

	if(!(argumentChecker & MAX_EDGE_COUNT_PER_NODE))
	{
		maxEdgeCountPerNode = numberOfNodes - 1;
	}

	if(argumentChecker & NORMAL_DISTRIBUTION)
	{
		if(!(argumentChecker & NORMAL_DISTRIBUTION_MEAN))
		{
			std::cout << "Missing mean information for the normal distribution. Use \"-normalDistributionMean <your_value>\"." << std::endl;
			exit(-1);
		}
		else if(!(argumentChecker & NORMAL_DISTRIBUTION_STDDEV))
		{
			std::cout << "Missing standard deviation information for the normal distribution. Use \"-normalDistributionStddev <your_value>\"." << std::endl;
			exit(-1);
		}

		normalDistribution = std::normal_distribution<double>(normalDistributionMean, normalDistributionStddev);
	}
	else
	{
		uniformDistribution = std::uniform_int_distribution<unsigned int>(minEdgeCountPerNode, maxEdgeCountPerNode);
	}

	std::cout << "Each vertex will have between " << minEdgeCountPerNode << " and " << maxEdgeCountPerNode << " edges." << std::endl;

	FILE* outputFile;
	if(argumentChecker & BINARY_OUTPUT)
	{
		outputFile = fopen(outputFileName.c_str(), "wb");
		std::cout << "File opened in binary format." << std::endl;
	}
	else
	{
		outputFile = fopen(outputFileName.c_str(), "w");
		std::cout << "File opened in ASCII format." << std::endl;
	}

	if(outputFile == NULL)
	{
		std::cout << "Cannot open the output file: " << argv[2] << std::endl;
		exit(-1);
	}

	/////////////////////////////////
	// PROGRAM START
	/////////////////////////////////
	if(argumentChecker & BINARY_OUTPUT)
	{
		fwrite(&numberOfNodes, sizeof(unsigned int), 1, outputFile);
	}
	std::vector<unsigned int> exclusions;
	double numberOfEdgesTemp = 0.0;
	unsigned int numberOfEdges = 0;
	unsigned int iteratorNodes = 0;
	unsigned int iteratorEdges = 0;
	unsigned int totalNumberOfEdges = 0;

	// Sets the seed of the random number generator to have reproductable results.
	int nodeSeed = 0;
	srand(nodeSeed);
	unsigned int* buffer = (unsigned int*)malloc(sizeof(unsigned int) * 3);
	unsigned int buffer_size = 3;
	unsigned int chunk = (numberOfNodes - (numberOfNodes % 100))/100;

	std::cout.imbue(std::locale("")); // For the thousand separators.
	std::cout << "0% of the graph produced." << std::flush;
	for(unsigned int source = 0; source < numberOfNodes; source++)
	{
		if(source % chunk == 0)
		{
			std::cout << '\r' << ((double)source)/((double)numberOfNodes) * 100.0 << "% of the graph produced, weighting " << fileSize << "B." << std::flush;
		}
		// Reset the list of available indexes for neighbours
		exclusions.clear();
		if(argumentChecker & NORMAL_DISTRIBUTION)
		{
			numberOfEdgesTemp = (normalDistribution(generator));
			while(numberOfEdgesTemp < 1.0 || numberOfEdgesTemp > numberOfNodes - 1)
			{
				numberOfEdgesTemp = (normalDistribution(generator));
			}
			numberOfEdges = (unsigned int)(numberOfEdgesTemp);
			if(numberOfEdges < minEdgeCountPerNode)
			{
				numberOfEdges = minEdgeCountPerNode;
			}
			else if(numberOfEdges > maxEdgeCountPerNode)
			{
				numberOfEdges = maxEdgeCountPerNode;
			}
		}
		else
		{
			numberOfEdges = uniformDistribution(generator);
		}

		totalNumberOfEdges += numberOfEdges;
		if(buffer_size < numberOfEdges + 2)
		{
			buffer = (unsigned int*)realloc(buffer, sizeof(unsigned int) * (numberOfEdges + 2));
			buffer_size = numberOfEdges + 2;
		}
		buffer[0] = source;
		buffer[1] = numberOfEdges;
#ifdef DEBUG_VERSION
		std::cout << "Vertex " << buffer[0] << " will have " << numberOfEdges << " neighbours." << std::endl;
#endif
		iteratorEdges = 2;
		while(iteratorEdges < numberOfEdges + 2)
		{
			buffer[iteratorEdges] = nextID(exclusions);
			exclusions.push_back(buffer[iteratorEdges]);
#ifdef DEBUG_VERSION
			std::cout << source << "->" <<  buffer[iteratorEdges] << std::endl;
#endif
			iteratorEdges++;
		}
		if(argumentChecker & BINARY_OUTPUT)
		{
			fwrite(buffer, sizeof(unsigned int), buffer_size, outputFile);
			fileSize += buffer_size * sizeof(unsigned int);
		}
		else
		{
			fprintf(outputFile, "%u\t%u", buffer[0], buffer[1]);
			for(unsigned int i = 2; i < buffer_size; i++)
			{
				fprintf(outputFile, " %u", buffer[i]);
			}
			fprintf(outputFile, "\n");
		}
	}
	std::cout << '\r' << "100% of the graph produced, weighting " << fileSize << "B." << std::endl;
	std::cout << "Now, closing the file... this may take a few moments..." << std::endl;
	free(buffer);
	fclose(outputFile);
	std::cout << "File closed. Graph summary: " << numberOfNodes << " nodes created, with a total of " << totalNumberOfEdges << " edges." << std::endl;

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
