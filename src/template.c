/*
 * Hi :)
 * In this file, you will be told how to structure your code to use my_pregel.
 * Time estimated: 5 minutes.
 */

/* 
 * 1) Tell my_pregel the vertex identifier type VERTEX_ID
 * To begin with, my_pregel must know the type you use to identify the vertices,
 * known as VERTEX_ID. Let imagine you identify your vertices with an "int",
 * you write after this comment block:
 * typedef int VERTEX_ID;
 */

// -------> INSERT YOUR TYPEDEF FOR VERTEX_ID HERE <-------

/*
 * 2) Remove the error pragma
 * Once 1) is done, you can delete the pragma below. This pragma was 
 * meant to ensure you could not compile this template without giving all the
 * information that are needed by my_pregel.
 */
#error VERTEX_ID undefined. Please read the comments in the file you try to \
compile; they explain how to do it.

/*
 * 3) Tell my_pregel the vertex message type MESSAGE_TYPE
 * Similarly to VERTEX_ID, my_pregel must know the type of the messages that
 * your vertices will exchange. If you want your vertices to exchange several
 * types of messages, just use a union structure. Let imagine your vertices 
 * exchange "int" values, you write after this comment block:
 * typedef int MESSAGE_TYPE;
 */

// -------> INSERT YOUR TYPEDEF FOR MESSAGE_TYPE HERE <-------

/*
 * 4) Remove the error pragma
 * Once 3) is done, you can delete the pragma below. This pragma was 
 * meant to ensure you could not compile this template without giving all the
 * information that are needed by my_pregel.
 */
#error MESSAGE_TYPE undefined. Please read the comments in the file you try to \
compile; they explain how to do it.

/*
 * 5) Include the preamble of my_pregel.
 * All the prototypes of my_pregel functions and my_pregel global variables 
 * (such as "superstep" for instance) are defined in my_pregel_preamble.h
 * so you must include it. This file also requires VERTEX_ID and MESSAGE_TYPE
 * to be defined, this is why we have done 1), 2), 3) and 4). At this point, 
 * VERTEX_ID and MESSAGE_TYPE must be defined, so we can include the preamble.
 */
#include "my_pregel_preamble.h"

/*
 * 6) Define the structure of the vertex
 * This is where you tell my_pregel the structure of your vertices. There are
 * two constraints though:
 * - It must be a struct vertex_t.
 * - It must contain the macro VERTEX_STRUCTURE.
 * The name of the structure is fixed by design, and the macro VERTEX_STRUCTURE
 * contains all the vertices attributes that are used by my_pregel behind the
 * scenes. For example, it contains the structures of the message queues.
 * IMPORTANT: The vertex identifier is already contained in VERTEX_STRUCTURE;
 * it is called "id" and has the type VERTEX_ID. So there is no need to put it
 * again. However, if you want your vertices to store values, you must give
 * them here. In many algorithms, we want the vertices to store an intermediate
 * value for communications. Let imagine this value is of type MESSAGE_TYPE,
 * then you write after this comment block:
 * struct vertex_t
 * {
 *     VERTEX_STRUCTURE
 *     MESSAGE_TYPE value;
 * };
 **/

// ------> INSERT YOUR VERTEX_T STRUCTURE HERE <------

/*
 * 7) Remove the error pragma
 * Once again, now that you must have defined the struct vertex_t, you can safely
 * remove the pragma below that is meant to prevent you from compiling a my_pregel
 * program without having given all the information needed first.
 */
#error struct vertex_t undefined. Please read the comments in the file you try \
to compile; they explain how to do it.

/*
 * 8) Define mandatory user functions.
 * There are three functions that you must define:
 * - compute: the function that is called on each vertex at each superstep.
 * - deserialise_vertex: the function called at the beginning to load the vertices
 * from the input file.
 * - serialise_vertex: the function called at the end of the run to output the
 * vertices in a file.
 */
void compute(struct vertex_t* v)
{
	// Put your definition here. When it is done, remove the pragma below.
	#error compute function undefined. Please read the comments in the file you \
try to compile; they explain how to do it.
}

void deserialiseVertex(FILE* f, struct vertex_t* v)
{
	// Put your definition here. When it is done, remove the pragma below.
	#error deserialise_vertex function undefined. Please read the comments in \
the file you try to compile; they explain how to do it.
}

/*
 * 9) Define optional functions.
 * There is one optional function in my_pregel; combine. The combiner is used
 * to reduce the number of messages that are stored on each vertex. You are
 * strongly encouraged to use it if your problem allows to, because it greatly
 * improves my_pregel performance.
 * If you do not use a combiner, the compilation will yield a warning to remind
 * you to use one if you can, as it greatly improves the performance of
 * my_pregel. However, if you do not want / cannot use one, you can remove the
 * warning pragma in the combine function below.
 * IMPORTANT: If you choose to define a combiner, append the compilation flag 
 * "-DUSE_COMBINER" to your makefile or compilation command so that my_pregel
 * can know it at compile and take the version optimised for combiners.
 */
void combine(MESSAGE_TYPE* a, MESSAGE_TYPE* b)
{
	(void)(a);
	(void)(b);
	#warning No combiner defined. The performance of my_pregel greatly benefits \
from the use of combiners, so it your solution can be designed with the use of \
combiners, please do.
}

/*
 * 10) Write your main.
 * At this stage, you have only one function you are required to call:
 * - run: the function that will start the my_pregel program.
 */
int main(int argc, char* argv[])
{
	/* Here is the function you must call:
	 * run(FILE* f, unsigned int number_of_vertices);
 	 * You must pass it a pointer on the input file, and the total number of
	 * vertices contained in that file.
 	 * NOTE: You are in charge of making sure the input file is successfully open.
     * Once it is called, you can remove the pragma below.
	 **/
	#error The run function is never called. Please read the comments in the \
file you try to compile; they explain how to do it.
}

/*
 * 11) Include the postamble.
 * The last step, extremely important, consists in including the postamble that
 * contains all the function definitions. 
 */
#include "my_pregel_postamble.h"

/*
 * 12) Compile your file
 * If you have reached this point and followed all the above, your file must
 * be ready for compilation. Enjoy :)
 */
