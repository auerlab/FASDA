#!/bin/sh -e


##########################################################################
#   Function description:
#       Pause until user presses return
##########################################################################

pause()
{
    local junk
    
    printf "Press return to continue..."
    read junk
}

# Abundances
kallisto=Results/07-fasda-kallisto/SNF2-all-norm-03.tsv
hisat2=Results/10-fasda-hisat/SNF2-all-norm-03.tsv

kallisto=Results/06-kallisto-quant/WT-1/abundance.tsv
hisat2=Results/09-hisat-align/WT-1-abundance.tsv
for transcript in $(awk '{ print $1 }' $kallisto); do
    echo '==='
    printf "Kallisto: "
    grep $transcript $kallisto
    printf "Hisat2:   "
    grep $transcript $hisat2
    k=$(awk -v t=$transcript '$1 == t { print $2 + $3 + $4 }' $kallisto)
    h=$(awk -v t=$transcript '$1 == t { print $2 + $3 + $4 }' $hisat2)
    printf "$h / ($k + .0000001)\n" | bc -l
done | more
exit

# Fold-changes
kallisto=Results/13-fasda-kallisto/chondro-time1-time2-FC.txt
hisat2=Results/20-fasda-fc-hisat2/chondro-time1-time2-FC.txt
for transcript in $(awk '{ print $1 }' $kallisto | head -20); do
    echo $transcript
    grep $transcript $kallisto
    grep $transcript $hisat2
done
