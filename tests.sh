#!/bin/bash

graph_directory='graphs';
reference_outputs_directory='reference_outputs';

failure_prefix="\033[31m[FAILURE]\033[0m";
success_prefix="\033[32m[SUCCESS]\033[0m";

return_code=0;

# For each benchmark
for b in pagerank cc sssp; do
	# Run each version
	for v in bin/${b}*; do
		v_without_bin=`echo ${v} | cut -d '/' -f2`;
		# On each graph
		for g in DBLP; do
			configuration="${v_without_bin} on ${g}";
			# Check the graph is present
			graph_path="${graph_directory}/${g}";
			if [ ! -f "${graph_path}.adj" ] || [ ! -f "${graph_path}.config" ] || [ ! -f "${graph_path}.adj" ]; then
				echo -e "${failure_prefix} ${configuration} The reference graph has not been found. Or only partially.";
				return_code=-1;
			else
				# Check the reference output is present
				reference_output="${reference_outputs_directory}/${b}_${g}.txt";
				if [ ! -f "${reference_output}" ]; then
					echo -e "${failure_prefix} ${configuration} The reference output has not been found."
					return_code=-1;
				else
					# Run the corresponding version of the corresponding benchmark on the corresponding graph
					supersteps_output='.tmp_output';
					${v} ${graph_path} out.txt 4 | grep Superstep | cut -d ' ' -f6 > ${supersteps_output};

					# Get the number of supersteps on the reference
					supersteps_count_reference=`cat ${reference_output} | wc -l`;
					supersteps_count=`cat ${supersteps_output} | wc -l`;

					if [ "${supersteps_count_reference}" -eq "${supersteps_count}" ]; then
						if [ -n "$(cmp ${reference_output} ${supersteps_output})" ]; then
							echo -e "${failure_prefix} ${configuration}: the number of active vertices at the end of each superstep diverge.";
							return_code=-1;
						else
							echo -e "${success_prefix} ${configuration}";
						fi
					else
						echo -e "${failure_prefix} ${configuration} Failure: different number of supersteps (${supersteps_count_reference} for reference, ${supersteps_count} for yours).";
						return_code=-1;
					fi
				fi
			fi
		done
	done
done
exit ${return_code};
