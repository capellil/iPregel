[![Build Status](https://travis-ci.com/capellil/iPregel.svg?branch=master)](https://travis-ci.com/capellil/iPregel)
# iPregel, light but fast

## Table of contents
- [What is iPregel?](#what-is-ipregel)
- [Getting started](#getting-started)
    - [Dependencies](#dependencies)
    - [Installation and setup](#installation-and-setup)
    - [Applications provided](#applications-provided)
        - [Compile](#compile)
        - [Run](#run)
- [Write your own application](#write-your-own-application)
    - [Types to define](#types-to-define)
    - [Functions to define](#functions-to-define)
    - [Interface](#interface)
    - [Tell your needs](#tell-your-needs)
    - [Pick the best version](#pick-the-best-version)
    - [Input graph](#input-graph)
- [History](#history)
- [Publications](#publications)

## What is iPregel?

In a nutshell, **iPregel** is a shared-memory framework for vertex-centric graph processing, using in-memory execution. Concretely, it is written in [C](https://en.wikipedia.org/wiki/C_(programming_language)), parallelised with [OpenMP](https://www.openmp.org) and totals a bit less than 2,000 lines of code at the time of writing. The source code documentation, written using [Doxygen](http://www.doxygen.nl), represents 30% of the total source code length.

## Getting started
### Dependencies

Technically speaking, **iPregel** has three dependencies: ```make```, a C compiler that supports OpenMP (```gcc``` is fine) and a C++ compiler (```g++``` is fine). Note that it is extremely unlikely your computer misses one of those.

*Note*: iPregel can use the XTHI utility (borrowed from https://github.com/Wildish-LBL/SLURM-demo/blob/master/xthi.c) to report the placement of OpenMP threads. However, this utility relies on the use of `cpuset_to_cstr`, which is a linux-only code. This is why the use of the XTHI utility is enabled on linux builds only.

### Installation and setup

Cloning the repository is all that has to be done.

```
git clone https://github.com/capellil/iPregel iPregel;
```

[Go back to table of contents](#table-of-contents)

### Applications provided

You will find in the benchmarks folder the vertex-centric version of three classic algorithms:
- [Connected components](https://en.wikipedia.org/wiki/Component_(graph_theory))
- [PageRank](https://en.wikipedia.org/wiki/PageRank)
- [Shortest-Single Source Path](https://www.techiedelight.com/single-source-shortest-paths-dijkstras-algorithm/)

#### Compile
The makefile is already designed to compile all three applications mentioned above. In addition, it also compiles every possible version of each application when they are compatible with multiple iPregel versions. Issuing ```make``` is all the user has to do.

[Go back to table of contents](#table-of-contents)

#### Run
All applications have been designed so they can be executed as follows:

```
./<application> <inputGraph> <outputFile> <numberOfThreads>
```

[Go back to table of contents](#table-of-contents)

## Write your own application

``` c
// Define types
// ...

#include "iPregel.h"

// Define user functions
// ...
```

### Types to define
Unlike common software, iPregel almost has no hard-coded types. This decision is motivated by the will to keep the memory footprint as low as possible. For instance, the maximal number of out-neighbours for any given vertex may be 100 for a given graph, for which an unsigned char suffices, and going up to trillions for another graph, for which an unsigned long int should do. Therefore, instead of hard-coding the largest type existing to cover all possible cases, iPregel lets the user define the type they need. In total, 4 types must be defined by the user:

| Type to define | Description |
| --- | --- |
| ```IP_VERTEX_ID_TYPE``` | The type to use for vertex identifiers. |
| ```IP_MESSAGE_TYPE``` | The type of message sent between vertices. If vertices may send different types of messages, you can use a union. |
| ```IP_NEIGHBOURS_COUNT_TYPE``` | The type to use to encode the number of neighbours of vertices. |
| ```IP_VALUE_TYPE``` | The type of the value that each vertex contains. Typically, this is the same type as that of the messages exchanged. |
| ```IP_EDGE_WEIGHT_TYPE``` | The type to use represent the edge weight. |

Here is an example snippet defining these defines:

```c
typedef unsigned int IP_VERTEX_ID_TYPE;
typedef IP_VERTEX_ID_TYPE IP_NEIGHBOUR_COUNT_TYPE;
typedef double IP_MESSAGE_TYPE;
typedef IP_MESSAGE_TYPE IP_VALUE_TYPE;
typedef IP_MESSAGE_TYPE IP_EDGE_TYPE; // <- if you have unweighted edges, you don't need this one
```

[Go back to table of contents](#table-of-contents)

### Functions to define

There are 3 functions that must be defined by the user:

| Function to define | Description |
| --- | --- |
| ```ip_compute``` | That's where you actual computation will take place. You can see it as your vertex ```main``` function if you want; it is the function that every active vertex will call at every iteration. |
| ```ip_combine``` | This is the function that will be called everytime a vertex receives a messages while already having one in its mailbox. This function will tell how to combine both messages: keep the min? keep the max? do the sum? etc... |
| ```ip_serialise_vertex``` | This is the function that will be called once the entire computation is finished. This function tells what information of a vertex needs to be stored into the output file; it will be called once, on every vertex. Note that the file, in which output the vertex information, must be already open by the user. |

```c
void ip_compute(struct ip_vertex_t* me) { ... }
void ip_combine(IP_MESSAGE_TYPE* a, IP_MESSAGE_TYPE b) { ... }
void ip_serialise_vertex(FILE* f, struct ip_vertex_t* v) { ... }
```

[Go back to table of contents](#table-of-contents)

### Interface
Although the documentation of iPregel covers all functions, this section conveniently introduces the functions that will help you develop your application. First, you have the functions that allow you to interact with the vertex being run:

| Vertex function | Description |
| --- | --- |
| ```ip_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE* m)``` | sends message ```m``` to vertex ```id```. |
| ```ip_broadcast(struct ip_vertex_t* v, IP_MESSAGE_TYPE* m)``` | sends the message ```m``` to all neighbours of vertex ```v```. |
| ```ip_vote_to_halt(struct ip_vertex_t* v)``` | vertex ```v``` votes to halt. |
| ```ip_has_message(struct ip_vertex_t* v)``` | returns true if the vertex v has a message in its inbox. |
| ```ip_get_next_message(struct ip_vertex_t* v, IP_MESSAGE_TYPE* m)``` | takes next message from inbox and puts it in ```m```. If no message left, does nothing. |

Second, you have the functions that allow you to get general information on the program.

| General function | Description |
| --- | --- |
| ```ip_get_superstep()``` | returns the current superstep number (0-indexed). |
| ```ip_is_first_superstep()``` | returns true if the current superstep is the superstep 0. False otherwise. |
| ```ip_get_vertices_count()``` | returns the total number of vertices in the graph. |

[Go back to table of contents](#table-of-contents)

### Tell your needs

One of the means that **iPregel** leverages to keep vertices as light as possible is to pack only attributes that will be needed during the computation. For instance, it prevents **iPregel** from packing vertices with incoming neighbour information if only outgoing neighbours are needed.

The means by which the user's needs are expressed is via defines. They can either be part of your source code or passed during the compilation command. (*Don't forget these are defines, that is, they are meant to be prepended with ```-D``` when passed as compilation flags.*)

| Define                         | Explanation                                                          |
| ------------------------------ | -------------------------------------------------------------------- |
| ```IP_NEEDS_IN_NEIGHBOURS_COUNT```   | Needs in-neighbours count.                                           |
| ```IP_NEEDS_IN_NEIGHBOUR_IDS```      | Needs in-neighbours identifiers.                                     |
| ```IP_NEEDS_IN_NEIGHBOUR_WEIGHTS```  | Needs in-neighbours weights.                                         |
| ```IP_NEEDS_OUT_NEIGHBOURS_COUNT```  | Needs out-neighbours count.                                          |
| ```IP_NEEDS_OUT_NEIGHBOUR_IDS```     | Needs out-neighbours identifiers.                                    |
| ```IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS``` | Needs out-neighbours weights.                                        |
| ```IP_WEIGHTED_EDGES```              | Indicates that edges have weights. If you indicate that in / out neighbours are unused, the edge weights will not be stored either. Also, if you indicate that in / out neighbour identifiers are unused, edge weights will not be stored because the user could not address them. |

[Go back to table of contents](#table-of-contents)

### Pick the best version

Unlike many software, **iPregel** does not rely on a one-size-fits-all design where a single implementation must cover all potential cases. Such an implementation cannot be simultaneously flexible enough so that it adapts to any kind of vertex-centric program and have optimisations tailored for each one. To counter that, **iPregel** does offer an implementation that works for all vertex-centric programs, but it also has multiple internal implementations; each being optimised for programs that expose certain properties.

The different implementations can be selected using the defines below. They can either be part of your source code or passed during the compilation command. (*Don't forget these are defines, that is, they are meant to be prepended with ```-D``` when passed as compilation flags.*)

| Define                         | Explanation                                                          |
| ------------------------------ | -------------------------------------------------------------------- |
| ```IP_USE_SPREAD```                  | Enable the spreading technique.                                      |
| ```IP_USE_SPINLOCK```                | Replace mutexes with spinlocks.                                      |
| ```IP_USE_SINGLE_BROADCAST```        | Communications exclusively use broadcasts.   

[Go back to table of contents](#table-of-contents)

### Input graph
The input graph passed is expected to be in the [binary format](https://github.com/jshun/ligra#input-format-for-ligra-applications-and-the-ligra-encoder), as used by Ligra. This graph, required to be static, can either be made of:
- **undirected edges**: in which case iPregel knows that the adjacency list it has for each vertex contains the out-neighbours, but also the in-neighbours of that vertex for that matter.
- **directed edges**: in which case iPregel knows that the adjacency list it has for each vertex only contains the out-neighbours. Therefore, iPregel will have to build the adjacency list of in-neighbours for each vertex.

As a consequence, iPregel must be told whether the graph is using directed or undirected edges. This information is expressed as part of the arguments passed to ```ip_init```.

[Go back to table of contents](#table-of-contents)

## History
**iPregel** has been developed by [Ludovic Capelli](https://www.linkedin.com/in/capellil/) during an internship at the [National Institute of Informatics](https://www.nii.ac.jp) in Tokyo, Japan. This internship took place thanks to the [International Internship  Program](https://www.nii.ac.jp/en/about/international/mouresearch/)  of  the  [National Institute of Informatics](https://www.nii.ac.jp), in 2017-2018, under the supervision of [Professor Hu](http://research.nii.ac.jp/~hu/). It was then supported by [Japan Society for the Promotion of Science](https://www.jsps.go.jp/english/) (grant number 17H06099).

Since 2018, it has been being developed by [Ludovic Capelli](https://www.linkedin.com/in/capellil/) as part of his PhD at [The University of Edinburgh](https://www.ed.ac.uk) under the supervision of [Dr Nick Brown](https://www.epcc.ed.ac.uk/about/staff/dr-nick-brown), [Dr Mark Bull](https://www.epcc.ed.ac.uk/about/staff/dr-mark-bull) and [Professor James Cheney](https://homepages.inf.ed.ac.uk/jcheney/). It is supported by the [UK Engineering and Physical Sciences Research Council](https://epsrc.ukri.org) (grant number EP/L01503X/1, [CDT in Pervasive Parallelism](http://web.inf.ed.ac.uk/infweb/student-services/cdt/pervasive-parallelism)).

[Go back to table of contents](#table-of-contents)

## Publications
1. Ludovic A. R. Capelli, Nick Brown and J. Mark Bull. 2019. "iPregel: Strategies to Deal with an Extreme Form of Irregularity in Vertex-Centric Graph Processing" in 2019 IEEE/ACM 9th Workshop on Irregular Applications: Architectures and Algorithms (IA3). IEEE, 2019. DOI: https://doi.org/10.1109/IA349570.2019.00013
1. Ludovic A. R. Capelli, Zhenjiang Hu, Timothy A. K. Zakian, Nick Brown and J. Mark Bull. 2019. "iPregel: Vertex-Centric Programmability Vs Memory Efficiency And Performance, Why Choose?" in Journal of Parallel Computing (PARCO'19), Volume 86, Pages 45-56. DOI: https://doi.org/10.1016/j.parco.2019.04.005.
1. Ludovic A. R. Capelli, Zhenjiang Hu, and Timothy A. K. Zakian. 2018. "iPregel: A Combiner-Based In-Memory Shared Memory Vertex-Centric Framework" in Proceedings of the 47th International Conference on Parallel Processing Companion (ICPP '18). ACM, New York, NY, USA, Article 33, 10 pages. DOI: https://doi.org/10.1145/3229710.3229719

[Go back to table of contents](#table-of-contents)
