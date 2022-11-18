#!/bin/sh -e

# Document software versions used for publication
uname -a
fastq-trim --version
pwd

# So gzip can keep up with fastq-trim
export GZIP=-1

# Allocate 3 cores per job, if possible
cores=$(parallel --number-of-cores)
jobs=$(($cores / 3))
if [ $jobs = 0 ]; then
    jobs=1
fi
printf "Cores = $cores  Jobs = $jobs\n"

trimmed_dir=Data/02-trim
mkdir -p $trimmed_dir
ls Data/Raw-renamed/*.fastq.gz \
    | parallel --max-procs $jobs ./trim1.sh $trimmed_dir

