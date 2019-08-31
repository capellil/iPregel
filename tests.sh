#!/bin/bash

benchmarks_directory='benchmarks';
graph_directory='graphs';
reference_outputs_directory='reference_outputs';

graph_output_suffix='graph_output.txt';
graph_output=".${graph_output_suffix}";
execution_output_suffix='execution_output.txt';
execution_output=".${execution_output_suffix}";

failure_prefix="\033[31m[FAILURE]\033[0m";
success_prefix="\033[32m[SUCCESS]\033[0m";

return_code=0;
specific_parameters='';

# For each benchmark
for b in pagerank cc sssp; do
	# Run each version
	for v in bin/${b}*; do
		v_without_bin=`echo ${v} | cut -d '/' -f2`;
		# On each graph
		for g in DBLP; do
			# Fetch benchmark-graph specific parameters
			if [ "${b}" == "sssp" ]; then
				# For SSSP, get the source_vertex
				specific_parameters=`cat ${benchmarks_directory}/sssp.c | grep reference | grep ${g} | cut -d '=' -f2`;
			fi

			configuration="${v_without_bin} on ${g}";
			# Check the graph is present
			graph_path="${graph_directory}/${g}";
			if [ ! -f "${graph_path}.adj" ] || [ ! -f "${graph_path}.config" ] || [ ! -f "${graph_path}.adj" ]; then
				echo -e "${failure_prefix} ${configuration}: the reference graph has not been found. Or only partially.";
				return_code=-1;
			else
				# Check the reference output is present
				execution_output_reference="${reference_outputs_directory}/${b}_${g}_${execution_output_suffix}";
				if [ ! -f "${execution_output_reference}" ]; then
					echo -e "${failure_prefix} ${configuration}: the reference file for execution output has not been found."
					return_code=-1;
				else
					# Run the corresponding version of the corresponding benchmark on the corresponding graph
					${v} ${graph_path} ${graph_output} 4 ${specific_parameters} | grep Superstep | cut -d ' ' -f6 > ${execution_output};
					
					if [ "$?" -eq "0" ]; then
						# Get the number of supersteps on the reference
						supersteps_count_reference=`cat ${execution_output_reference} | wc -l`;
						supersteps_count=`cat ${execution_output} | wc -l`;
						
						if [ "${supersteps_count_reference}" -eq "${supersteps_count}" ]; then
							graph_output_reference="${reference_outputs_directory}/${b}_${g}_${graph_output_suffix}";
							if [ ! -f "${graph_output_reference}" ]; then
								echo -e "${failure_prefix} ${configuration}: the reference file for graph output has not been found."
								return_code=-1;
							else
								if [ -n "$(cmp ${graph_output} ${graph_output_reference})" ]; then
									echo -e "${failure_prefix} ${configuration}: the output graphs generated diverge.";
									return_code=-1;
								else
									echo -e "${success_prefix} ${configuration}";
								fi
							fi
						else
							echo -e "${failure_prefix} ${configuration}: different number of supersteps (${supersteps_count_reference} for reference, ${supersteps_count} for yours).";
							return_code=-1;
						fi
					else
						echo "${failure_prefix} ${configuration}: a failure error code has been returned by the benchmark."
						return_code=-1;
					fi
				fi
			fi
		done
	done
done
exit ${return_code};
