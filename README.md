[![Build Status](https://travis-ci.com/capellil/iPregel.svg?token=fEMxpNKNrEnCWwNyxuq4&branch=master)](https://travis-ci.com/capellil/iPregel)
# iPregel, light but fast <in construction>

## Table of contents
- [Characteristics](#characteristics)
    - [Vertex-centric](#vertex-centric)
    - [Shared-memory](#shared-memory)
    - [In-memory](#in-memory)
    - [Combiner-based](#combiner-based)
    - [Multi-version design](#multi-version-design)
- [Guidelines](#guidelines)
    - [Defines to define](#defines-to-define)
    - [Input graph](#input-graph)
        - [Format](#format)
        - [Restrictions](#restrictions)
    - [Template application](#template-application)
        - [User-defined types](#user-defined-types)
        - [User-defined functions](#user-defined-functions)
    - [Interface](#interface)
- [Install](#install)
- [Run](#run)
- [Applications provided](#applications-provided)
- [History](#history)
- [Publications](#publications)

## Characteristics

### Vertex-centric
**Definition**

In a vertex-centric program, computations are expressed as if you were a vertex. These computations are then applied to all vertices in the graph.

**Advantages**
- Programs tend to be short and simple.
- Inherently offering a lot of parallelisation.

**Drawback**
- The user needs to think differently than their usual programming view.

[Go back to table of contents](#table-of-contents)

### Shared-memory
**Definition**

A program relying on shared-memory parallelism can use any number of cores available on your computer, but it cannot use several computers as per distributed programming.

**Advantage**
- Faster than their distributed-memory counterparts because there is no network communications involved, all communications are kept local.

**Drawback**
- The program is limited to the resources available on a single node, notably the amount of RAM and number of cores.

[Go back to table of contents](#table-of-contents)

### In-memory
**Definition**

A program with in-memory execution relies exclusively on RAM during execution. It contrasts with out-of-core execution programs, which can use disks (HDD / SSD...) to store data. 

**Advantage**
- Faster because there is no disk-IO involved; all memory interactions take place within the RAM. Of course, this is valid once the graph is loaded from the disk to the RAM, until then the program still needs to interact with the disk to load it.

**Drawback**
- Every single data, **including the graph itself**, is stored in RAM. In other words, an in-memory program must be able to make everything fit in RAM or it cannot run at all.

[Go back to table of contents](#table-of-contents)

### Combiner-based
**Definition**

A combiner is something that is invoked every time two messages need to be combined into a single one.

**Advantage**
- Avoids queuing messages, which requires dynamically resizeable structures and a lot of memory.

**Drawbacks**
- The user must inform the program of the combination operation to perform.
- The combination operation must be associative and commutative.

**Consequence**
- If a vertex receives a message, either it is the first it receives so it has 1 message in total, either it is not the first and that message will be combined with the existing one. Either way, a vertex will never have more than 1 message in total, so there is no need for dynamically-sized structure, which saves space.

[Go back to table of contents](#table-of-contents)

### Multi-version design
**Definition**

Rather than going for a one-size-fits-all design, iPregel was built upon multiple internal implementations, each of which is optimised for a certain set of assumptions.

**Advantage**
- Given a set of assumptions, iPregel can switch to its version that is the most optimised for that situation.

**Drawback**
- The user must give hints to iPregel, these will help it decide which internal implementation switch to.

**Consequences**
- Although iPregel has a default version picked, it must be given information to known which version to use. This is achieved with defines passed as compilation flags during compilation phase.
- To minimise the code redundancy mentioned above, the functions that are meant to be identical across versions are shared by all versions.
        
[Go back to table of contents](#table-of-contents)
        
## Guidelines

### Defines to define
The compilation flags mentioned above, that give information to iPregel about the assumptions that hold, are presented below. (*Don't forget these are defined, that is, they are meant to be prepended with ```-D``` when passed as compilation flags.*)

| Define                         | Explanation                                                          |
| ------------------------------ | -------------------------------------------------------------------- |
| ```IP_USE_SPREAD```                  | Enable the spreading technique.                                      |
| ```IP_USE_SPINLOCK```                | Replace mutexes with spinlocks.                                      |
| ```IP_USE_SINGLE_BROADCAST```        | Communications exclusively use broadcasts.                           |
| ```IP_NEEDS_IN_NEIGHBOURS_COUNT```   | Needs in-neighbours count.                                           |
| ```IP_NEEDS_IN_NEIGHBOUR_IDS```      | Needs in-neighbours identifiers.                                     |
| ```IP_NEEDS_IN_NEIGHBOUR_WEIGHTS```  | Needs in-neighbours weights.                                         |
| ```IP_NEEDS_OUT_NEIGHBOURS_COUNT```  | Needs out-neighbours count.                                          |
| ```IP_NEEDS_OUT_NEIGHBOUR_IDS```     | Needs out-neighbours identifiers.                                    |
| ```IP_NEEDS_OUT_NEIGHBOUR_WEIGHTS``` | Needs out-neighbours weights.                                        |
| ```IP_ID_OFFSET```                   | Indicates the the vertex identifiers start at 0 or to another value. |
| ```IP_WEIGHTED_EDGES```              | Indicates that edges have weights. If you indicate that in / out neighbours are unused, the edge weights will not be stored either. Also, if you indicate that in / out neighbour identifiers are unused, edge weights will not be stored because the user could not address them. |

[Go back to table of contents](#table-of-contents)

### Input graph
#### Format
The graph is expected to be in the [binary format](https://github.com/jshun/ligra#input-format-for-ligra-applications-and-the-ligra-encoder), as used by Ligra. The graph can either be made of:
- **undirected edges**: in which case iPregel knows that the adjacency list it has for each vertex contains the out-neighbours, but also the in-neighbours of that vertex for that matter.
- **directed edges**: in which case iPregel knows that the adjacency list it has for each vertex only contains the out-neighbours. Therefore, iPregel will have to built the adjacency list of in-neighbour for each vertex.

As a consequence, iPregel must be told whether the graph is using directed or undirected edges. This information is expressed as part of the arguments passed to ```ip_init```.

#### Restrictions
- **Combiners-based**: being combiner-based, iPregel requires the user to define a combiner.
- **Static graphs**: it is assumed that graphs will not be altered during execution, if the user decides to do so, it is an undefined behaviour.
- **Integral vertex identifiers**: vertex identifiers are required to be integral numbers. For most graphs, it is already the case, otherwise it is very likely that you can afford to abstract the real identifiers with integral numbers. It is considered to provide a feature in iPregel that would abstract identifiers on-the-fly, freeing the user from having to do any pre-processing on their own.
- **Contiguous identifiers**: the vertex identifiers - which are integral numbers - must be contiguous. That means there is no "hole" between a vertex identifier to the next. However, it is allowed that vertex identifiers do not start at 0. Allowed graphs: {0, 1, 2, 3} or {342, 343, 344, 345}. Not allowed graphs: {0, 1, 2, **4**} or {342, 343, 344, **346**}.

[Go back to table of contents](#table-of-contents)

### Template application

``` c
// Define types
typedef unsigned int IP_VERTEX_ID_TYPE;
typedef IP_VERTEX_ID_TYPE IP_NEIGHBOUR_COUNT_TYPE;
typedef double IP_MESSAGE_TYPE;
typedef IP_MESSAGE_TYPE IP_VALUE_TYPE;
#include "iPregel.h"

// Define user functions
void ip_compute(struct ip_vertex_t* me) { ... }
void ip_combine(IP_MESSAGE_TYPE* a, IP_MESSAGE_TYPE b) { ... }
void ip_serialise_vertex(FILE* f, struct ip_vertex_t* v) { ... }
```

#### User-defined types
Unlike common software, iPregel almost has no hard-coded types. This decision is motivated by the will to keep the memory footprint as low as possible. Indeed, variable encoding the number of out-neighbours of a vertex for instance may require any number of bytes. So, instead of taking the largest type existing to cover all possible cases, iPregel lets the user define the type they need so it uses the fittest. In total, 4 types must be defined by the user:

- **IP_VERTEX_ID_TYPE**: The type to use for vertex identifiers.
- **IP_MESSAGE_TYPE**: The type of message sent between vertices. If vertices may send different types of messages, you can use a union.
- **IP_NEIGHBOURS_COUNT_TYPE**: The type to use to encode the number of neighbours of vertices.
- **IP_EDGE_WEIGHT_TYPE**: The type to use represent the edge weight.

[Go back to table of contents](#table-of-contents)

#### User-defined functions

- Compute
- Combiner
- Dumping vertices

[Go back to table of contents](#table-of-contents)

### Interface
Although the documentation of iPregel covers all functions coded, the few functions given below represent the core one that every application is likely to require:   

| Function signature | Function explanation |
| --- | --- |
| ```ip_send_message(IP_VERTEX_ID_TYPE id, IP_MESSAGE_TYPE* m)``` | sends message ```m``` to vertex ```id```. |
| ```ip_broadcast(struct ip_vertex_t* v, IP_MESSAGE_TYPE* m)``` | sends the message ```m``` to all neighbours of vertex ```v```. |
| ```ip_vote_to_halt(struct ip_vertex_t* v)``` | vertex ```v``` votes to halt. |
| ```ip_get_superstep()``` | returns the current superstep number (0-indexed). |
| ```ip_is_first_superstep()``` | returns true if the current superstep is the superstep 0. False otherwise. |
| ```ip_get_vertices_count()``` | returns the total number of vertices. |
| ```ip_has_message(struct ip_vertex_t* v)``` | returns true if the vertex v has a message in its inbox. |
| ```ip_get_next_message(struct ip_vertex_t* v, IP_MESSAGE_TYPE* m)``` | takes next message from inbox and puts it in ```m```. If no message left, does nothing. |

[Go back to table of contents](#table-of-contents)

## Install
```
sudo apt-get install -y make gcc g++;
git clone https://github.com/capellil/iPregel iPregel;
cd iPregel;
export IP_ID_OFFSET=0;
make
```

[Go back to table of contents](#table-of-contents)

## Run
All binaries have been coded to be executed as follows:

```
./<application> <inputGraph> <outputFile> <numberOfThreads>
```

[Go back to table of contents](#table-of-contents)

## Applications provided

- Connected components
- PageRank
- Shortest-Single Source Path

[Go back to table of contents](#table-of-contents)

## History
iPregel has been developed by Ludovic Capelli during an internship at the National Institute of Informatics in Tokyo, Japan, in 2017-2018, under the supervision of Professor Hu.
Since 2018, it has been being developed by Ludovic Capelli as part of his PhD at the University of Edinburgh.

[Go back to table of contents](#table-of-contents)

## Publications
1. Ludovic A. R. Capelli, Zhenjiang Hu, Timothy A. K. Zakian, Nick Brown and J. Mark Bull. 2019. iPregel: Vertex-Centric Programmability Vs Memory Efficiency And Performance, Why Choose? In Journal of Parallel Computing (PARCO'19), Volume 86, Pages 45-56. DOI: https://doi.org/10.1016/j.parco.2019.04.005.
1. Ludovic A. R. Capelli, Zhenjiang Hu, and Timothy A. K. Zakian. 2018. iPregel: A Combiner-Based In-Memory Shared Memory Vertex-Centric Framework. In Proceedings of the 47th International Conference on Parallel Processing Companion (ICPP '18). ACM, New York, NY, USA, Article 33, 10 pages. DOI: https://doi.org/10.1145/3229710.3229719

[Go back to table of contents](#table-of-contents)
