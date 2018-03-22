# iPregel, light and fast <in construction>
## Straightforward version
If you are here to quickly get iPregel, it starts here, otherwise go to next section to get more details about this project.
### Everything in 1 command.
This command clones the code in the repository,
sets up the environment variables needed and builds everything (code + all benchmarks + all versions for each benchmark).
```
sudo apt-get install -y make gcc && git clone https://bitbucket.org/capellil/nii_my_pregel nii_my_pregel && cd nii_my_pregel && export IP_ID_OFFSET=0 && export OMP_NUM_THREADS=2 && make
```
## Detailed version
Below is a description of iPregel, where you will learn everything about it. First of all, iPregel is a combiner-based shared-memory in-memory vertex-centric framework. What does it mean?
### Characteristics of iPregel
- **Vertex-centric:** 
    - What: computations are expressed as if you were a vertex.
    - Advantage: very easy to write. Lot of parallelisation possible.
    - Drawback: need to think differently than the usual programming view.
- **Pure shared-memory:**
    - it uses the multiple cores available, but is *single-node* (by opposition to distributed-memory systems which can use more than one computer).
    - Advantage: faster because there is no network communications involved, they are all local.
    - Drawback: limited amount of resources (number of cores, RAM) that one can get from a single node.
- **In-memory:**
    - iPregel uses only RAM during execution, by opposition to 
    - Advantage: faster because there is no disk-IO involved.
    - Drawback: everything must fit in the RAM given, **including the graph itself**.
- **Combiner-based**
    - Definition: a combiner is something that is invoked every time two messages need to be combined into a single one. In the context of iPregel, it means 
    - Why
- **Multi-version design:**
### Aims
- Observation frameworks can use up to 800GB of memory for a graph of just 28GB, so there must be a lot of efforts doable in minimising a framework footprint. iPregel uses only the strict minimum of memory.
### Guidelines in using iPregel
#### Restrictions on graphs
- Combiners required
- Static graphs
- Contiguous identifiers
- Vertex identifiers being integral numbers
#### Defines to define
Done at compile time because the entire code can be optimised in a way that would not be possible otherwise.
Don't forget to prepend the defines given below with "-D" during the compilation phase.

| Define                      | Explanation                                                          |
| --------------------------- | -------------------------------------------------------------------- |
| IP_USE_SPREAD               | Enable the spreading technique.                                      |
| IP_USE_SPINLOCK             | Replace mutexes with spinlocks.                                      |
| IP_USE_SINGLE_BROADCAST     | Communications exclusively use broadcasts.                           |
| IP_UNUSED_IN_NEIGHBOURS     | Does not need in-neighbours.                                         |
| IP_UNUSED_OUT_NEIGHBOURS    | Does not need out-neighbours.                                        |
| IP_UNUSED_OUT_NEIGHBOUR_IDS | Does not need out-neighbours identifiers.                            |
| IP_ID_OFFSET                | Indicates the the vertex identifiers start at 0 or to another value. |
| FORCE_DIRECT_MAPPING        | No offset applied, possible wasted elements.                         |
| IP_WEIGHTED_EDGE            | Indicates that edges have weights.                                   |

### Run
./<benchmark> <inputGraph> <outputFile>
### Examples
- Hashmin
- PageRank
- SSSP
#### User-defined types
4 types must be defined by the user and are used internally by iPregel since iPregel does not rely on hard-coded types to be more memory-efficient.
#### User-defined functions
- Loading vertices
- Compute
- Combiner
- Dumping vertices
#### Interface
- ip_get_vertices_count(): returns the number of vertices.
### More
#### Author
iPregel has been developed by Ludovic Capelli during internship at the National Institute of Informatics in Tokyo, Japan, in 2017-2018.
#### Co-authors
There are people who did not write the code but gave advice, comments and directions that helped develop iPregel. Here they are:
- Professor Zhenjiang Hu
- (Future Dr) Tim Zakian