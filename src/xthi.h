#ifndef XTHI_H_INCLUDED
#define XTHI_H_INCLUDED

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <omp.h>

// Borrow from https://github.com/Wildish-LBL/SLURM-demo/blob/master/xthi.c
// Itself borrowed from util-linux-2.13-pre7/schedutils/taskset.c
static char *cpuset_to_cstr(cpu_set_t *mask, char *str)
{
  char *ptr = str;
  int i, j, entry_made = 0;
  for (i = 0; i < CPU_SETSIZE; i++) {
    if (CPU_ISSET(i, mask)) {
      int run = 0;
      entry_made = 1;
      for (j = i + 1; j < CPU_SETSIZE; j++) {
        if (CPU_ISSET(j, mask)) run++;
        else break;
      }
      if (!run)
        sprintf(ptr, "%d,", i);
      else if (run == 1) {
        sprintf(ptr, "%d,%d,", i, i + 1);
        i++;
      } else {
        sprintf(ptr, "%d-%d,", i, i + run);
        i += run;
      }
      while (*ptr != 0) ptr++;
    }
  }
  ptr -= entry_made;
  *ptr = 0;
  return(str);
}

void report_placement()
{
	cpu_set_t coremask;
	char clbuf[7 * CPU_SETSIZE];
	memset(clbuf, 0, sizeof(clbuf));

	printf("+--------+---------------+\n");
	printf("| THREAD | CORE AFFINITY |\n");
	printf("+--------+---------------+\n");

	#pragma omp parallel private(coremask, clbuf)
	{
		int thread = omp_get_thread_num();
		for(int i = 0; i < omp_get_num_threads(); i++)
		{
			if(thread == i)
			{
				(void)sched_getaffinity(0, sizeof(coremask), &coremask);
				cpuset_to_cstr(&coremask, clbuf);
				printf("| %6d | %13s |\n", thread, clbuf);
			}
			#pragma omp barrier
		}
	}

	printf("+--------+---------------+\n");
}

#endif // XTHI_H_INCLUDED
