#!/bin/bash --login

#SBATCH -D /home/nx01/nx01/capellil
# File for standard output
#SBATCH -o outputs_iPregel/OUT_25_500_pull_nvram_only_1N_1P_1T.log
# File for error output
#SBATCH -e outputs_iPregel/ERR_25_500_pull_nvram_only_1N_1P_1T.log
# Name of the job
#SBATCH -J i_1_1_1

# Number of nodes requested
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1      ##This should be scheduled automatically
#SBATCH --cpus-per-task=1      ##This should be scheduled automatically
#SBATCH --hint=nomultithread

# Walltime requested HH:MM:SS
#SBATCH --time=10:00:00

# Ask for the node set as 3TB of NVRAM as memory pool: 3tb-2lm for all 3TB as memory mode, normal for the rest
#SBATCH -p normal
 
# Use the node in app-mode: 1LM:1000 or memory-mode 2LM:1000
#SBATCH --nvram-options=1LM:1000

#export HFI_UNIT=0

#export PSM1_DEVICES="self,hfi,shm"
#export PSM1_MULTIRAIL=1
#export PSM1_MULTIRAIL_MAP=0:1,1:1

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PROC_BIND=TRUE
export OMP_PLACES=threads
export IPREGEL_GRAPH=~/graphs/25_500

#Execution command

# Module for the libvmem library
module load pmdk;

# Pagerank PUSH-mode
#srun ~/iPregel/bin/pagerank ${IPREGEL_GRAPH} out_25_500_iPregel_push_1N_1P_1T.txt ${OMP_NUM_THREADS}

# Pagerank PULL-mode
rmdir /mnt/pmem_fsdax0/temp_pmem
rmdir /mnt/pmem_fsdax1/temp_pmem
mkdir /mnt/pmem_fsdax0/temp_pmem
mkdir /mnt/pmem_fsdax1/temp_pmem
srun ~/iPregel/bin/pagerank_single_broadcast ${IPREGEL_GRAPH} out_25_500_iPregel_pull_nvram_only_1N_1P_1T.txt ${OMP_NUM_THREADS}
rmdir /mnt/pmem_fsdax0/temp_pmem
rmdir /mnt/pmem_fsdax1/temp_pmem
