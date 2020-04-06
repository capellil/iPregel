#!/bin/bash --login

#SBATCH -D /home/nx01/nx01/capellil
# File for standard output
#SBATCH -o outputs_iPregel/OUT_28_500_Pull_Static_1N_1P_24T.log
# File for error output
#SBATCH -e outputs_iPregel/ERR_28_500_Pull_Static_1N_1P_24T.log
# Name of the job
#SBATCH -J i_1_1_24

# Number of nodes requested
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1      ##This should be scheduled automatically
#SBATCH --cpus-per-task=24      ##This should be scheduled automatically
#SBATCH --hint=nomultithread

# Walltime requested HH:MM:SS
#SBATCH --time=22:00:00

# Ask for the node set as 3TB of NVRAM as memory pool
#SBATCH -p 3tb-1lm
 
# Use the node in app-mode: 1LM:1000 or memory-mode 2LM:1000
#SBATCH --nvram-options=1LM:1000

#export HFI_UNIT=0

#export PSM1_DEVICES="self,hfi,shm"
#export PSM1_MULTIRAIL=1
#export PSM1_MULTIRAIL_MAP=0:1,1:1

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PROC_BIND=TRUE
export OMP_PLACES=threads

#Execution command

module load pmdk;

# DIP version of Pagerank PUSH-mode
#srun ~/iPregel/bin/pagerank ~/graphs/28_500 out_iPregel_push.txt ${OMP_NUM_THREADS}


# DIP version of Pagerank PULL-mode
mkdir /mnt/pmem_fsdax0/temp_pmem
mkdir /mnt/pmem_fsdax1/temp_pmem
srun ~/iPregel/bin/pagerank_single_broadcast ~/graphs/28_500 out_iPregel_pull.txt ${OMP_NUM_THREADS}
rmdir /mnt/pmem_fsdax0/temp_pmem
rmdir /mnt/pmem_fsdax1/temp_pmem

