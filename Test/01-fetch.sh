#!/bin/sh -e

##########################################################################
#   Description:
#       Fetch Yeast sample data and create symlinks with descriptive names
#       
#   History:
#   Date        Name        Modification
#   2022-11-17  Jason Bacon Begin
##########################################################################

usage()
{
    printf "Usage: $0 samples\n"
    printf "Example: $0 10\n"
    exit 1
}


##########################################################################
#   Main
##########################################################################

if [ $# != 1 ]; then
    usage
fi
samples=$1

if [ $samples -lt 8 ]; then
    printf "Sample count must be at least 8 for Mann-Whitney U-test.\n"
    exit 1
fi

raw=Data/Raw
raw_renamed=Data/Raw-renamed
mkdir -p $raw $raw_renamed

for condition in WT SNF2; do
awk -v samples=$samples -v condition=$condition \
    '$2 == 1 && $3 == condition && $4 <= samples' \
    ERP004763_sample_mapping.tsv > $condition.tsv
    printf "$condition:\n"

    for sample in $(awk '{ print $1 }' $condition.tsv); do
	num=$(awk -v sample=$sample '$1 == sample { print $4 }' $condition.tsv)
	fq="$sample.fastq.gz"
	if [ ! -e $raw/$fq ]; then
	    printf "Downloading $sample = $condition-$num...\n"
	    if hostname | fgrep -q acadix.biz && ! which fasterq-dump; then
		# Local test platforms.  May not have sra-tools and pulling
		# from coral saves a lot of bandwidth.
		rsync --partial --progress \
		    coral:Prog/Src/fasda/Test/Data/Raw/$sample.fastq.gz $raw
	    else
		fasterq-dump --progress --force --outdir $raw $sample
		printf "Compressing...\n"
		gzip $raw/$sample.fastq
	    fi
	fi
	(cd $raw_renamed && ln -fs ../Raw/$fq $condition-$num.fastq.gz)
    done
    rm -f $condition.tsv
done
ls -l $raw
ls -l $raw_renamed
