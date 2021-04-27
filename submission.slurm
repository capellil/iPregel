#!/bin/bash --login

#SBATCH -D /home/nx01/nx01/capellil
# File for standard output
#SBATCH -o outputs_iPregel/OUT_%j.log
# File for error output
#SBATCH -e outputs_iPregel/ERR_%j.log
# Name of the job
#SBATCH -J iPregel

# Number of nodes requested
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --hint=nomultithread

# Walltime requested HH:MM:SS
#SBATCH --time=10:00:00

# Ask for the node set as 3TB of NVRAM as memory pool: 3tb-2lm for all 3TB as memory mode, normal for the rest
#SBATCH -p 3tb-2lm
 
# Use the node in app-mode: 1LM:1000 or memory-mode 2LM:1000
#SBATCH --nvram-options=2LM:1000

# SBATCH --dependency=after:64288

#export HFI_UNIT=0

#export PSM1_DEVICES="self,hfi,shm"
#export PSM1_MULTIRAIL=1
#export PSM1_MULTIRAIL_MAP=0:1,1:1

export OMP_NUM_THREADS=48
export OMP_PROC_BIND=TRUE
export OMP_PLACES=cores
export IPREGEL_GRAPH=~/graphs/K_25_500
export IPREGEL_OUTPUTS=/home/nx01/nx01/capellil/outputs_iPregel
export IPREGEL_SCHEDULE=static
export IPREGEL_CHUNK_SIZE=0

#Execution command

# Module for the libvmem library
#module load pmdk;

# Save the slurm script along with it
cat $0 > ${IPREGEL_OUTPUTS}/SLURM_${SLURM_JOB_ID}.log

# Pagerank PUSH-mode
#~/iPregel/bin/pagerank_32 ${IPREGEL_GRAPH} ${IPREGEL_OUTPUTS}/RESULT_${SLURM_JOB_ID}.txt ${OMP_NUM_THREADS} ${IPREGEL_SCHEDULE} ${IPREGEL_CHUNK_SIZE}

# Pagerank PULL-mode
~/iPregel/bin/pagerank_single_broadcast_32 ${IPREGEL_GRAPH} ${IPREGEL_OUTPUTS}/RESULT_${SLURM_JOB_ID}.txt ${OMP_NUM_THREADS} ${IPREGEL_SCHEDULE} ${IPREGEL_CHUNK_SIZE}
