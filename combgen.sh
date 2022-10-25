#!/bin/sh -e

##########################################################################
#   Synopsis:
#       
#   Description:
#       
#   Arguments:
#       
#   Returns:
#
#   Examples:
#
#   Files:
#
#   Environment:
#
#   See also:
#       
#   History:
#   Date        Name        Modification
#   2022-09-30  Jason Bacon Begin
##########################################################################

usage()
{
    printf "Usage: $0 k\n"
    exit 1
}


##########################################################################
#   Function description:
#       
#   Arguments:
#       
#   Returns:
#       
#   History:
#   Date        Name        Modification
#   2022-09-30  Jason Bacon Begin
##########################################################################

print_indent()
{
    if [ $# != 1 ]; then
	printf "Usage: indent n\n"
	exit 1
    fi
    c=$1
    
    for space in $(seq 1 $((c - 1))); do
	printf " "
    done
}


##########################################################################
#   Function description:
#       
#   Arguments:
#       
#   Returns:
#       
#   History:
#   Date        Name        Modification
#   2022-09-30  Jason Bacon Begin
##########################################################################

gen_loop()
{
    local k=$1
    
    # Downsample set of all possible means for replicates > 5 so that
    # 10 P-values can be computed in under 0.2 seconds.  Increments were
    # determined by trial and error with ./pval 100 120 .3 N 1
    case $k in
    5)
	increment=2 # 2, 3: p-values mostly stable to 2 decimal places
	passes=3    # Exact P-value (inc=1, no srandom) = 0.393 ~1 sec
		    # 100 200 .7 5 1
	;;
    6)
	increment=4 # 4, 6: p-values mostly stable to 2 decimal places
	passes=6    # Exact P-value (inc=1, no srandom) = 0.117 ~2 min
		    # 100 200 .7 6 1
	;;
    7)
	increment=6 # 6, 3: p-values mostly stable to 2 decimal places
	passes=3    # Exact P-value (inc=1, no srandom) = 0.048 several hours
		    # 100 200 .7 7 1
	;;
    8)
	increment=9 # 9, 4: p-values mostly stable to 2 decimal places
	passes=4
	;;
    9)
	increment=16 # 16, 5: p-values not stable to 2 decimal places
	passes=5
	;;
    10)
	increment=20 # 20, 7: p-values not stable to 2 decimal places
	passes=7
	;;
    *)
	increment=1
	passes=1
	;;
    esac
    
    cat << EOM

/*
 *  Generate all combinations n choose $k and count FCs >= observed.
 *  This is much faster than generic algorithms for generating n choose
 *  k lists for any n and k.
 */

unsigned long   extreme_fcs$k(count_pair_t count_pairs[], unsigned long pair_count,
			double observed_fc,
			unsigned long *fc_count)

{
    // Using sample++ % sample_rate doesn't produce much gain
    // Go after loop increments instead
    unsigned long   fc_ge = 0, fc_le = 0,
		    increment = $increment, pass, count = 0;
    double          c2_sum, c1_sum, fc;
    
EOM
    # Variable defs
    printf "    unsigned long  "
    for c in $(seq 1 $((k - 1))); do
	printf "c%d, " $c
    done
    printf "c%d;\n\n" $k
    
    # Nested loop
    printf "    for (pass = 0; pass < $passes; ++pass)\n"
    printf "     for (c1 = 0; c1 < pair_count; c1 += increment)\n"
    for c in $(seq 2 $k); do
	print_indent $c
	if [ $increment -gt 1 ]; then
	    printf "     for (c$c = c$((c - 1)) + 1 + random() %% increment; c$c < pair_count; c$c += increment)\n"
	else
	    printf "     for (c$c = c$((c - 1)) + 1; c$c < pair_count; c$c += increment)\n"
	fi
    done
    
    # Body
    print_indent $c
    printf "     {\n"
    print_indent $c
    printf "         c2_sum =\n"
    for c2 in $(seq 1 $((k - 1))); do
	print_indent $c
	printf "             count_pairs[c$c2].c2_count +\n"
    done
    print_indent $c
    printf "             count_pairs[c$k].c2_count;\n"
    print_indent $c
    printf "         c1_sum =\n"
    for c2 in $(seq 1 $((k - 1))); do
	print_indent $c
	printf "             count_pairs[c$c2].c1_count +\n"
    done
    print_indent $c
    printf "             count_pairs[c$k].c1_count;\n"
    print_indent $c
    printf "         fc = c2_sum / c1_sum;\n"
    # print_indent $c
    # printf '             if ( count %% 100000000 == 0 ) fprintf(stderr, "%%lu\\r", count);\n'

    print_indent $c
    printf "         if ( fc >= observed_fc ) ++fc_ge;\n"
    print_indent $c
    printf "         else if ( fc <= 1.0 / observed_fc ) ++fc_le;\n"

    print_indent $c
    printf "         ++count;\n"
    print_indent $c
    printf "     }\n"
    
    # Closing braces
    cat << EOM
    printf("FCs > %0.3f = %-5lu     FCs < %0.3f = %-5lu\n",
	    observed_fc, fc_ge, 1.0 / observed_fc, fc_le);
    *fc_count = count;
    
    return fc_ge + fc_le;
}
EOM
}


##########################################################################
#   Main
##########################################################################

cat << EOM
/*
 *  Generated by combgen.sh.  DO NOT EDIT.
 */

#include <stdio.h>
#include <stdlib.h>
#include "exact-p-val.h"

EOM
# FC-count choose 11 overflows a 64-bit integer
max_reps=10
for c in $(seq 2 $max_reps); do
    gen_loop $c
done

cat << EOM


unsigned long   extreme_fcs_count(count_pair_t count_pairs[], unsigned long pair_count,
		      unsigned long replicates, double observed_fc,
		      unsigned long *fc_count)

{
    static unsigned long (*extreme_fcs_funcs[])(count_pair_t count_pairs[],
				    unsigned long pair_count,
				    double observed_fc,
				    unsigned long *fc_count) =
    {
EOM

for c in $(seq 2 $((max_reps - 1))); do
    printf "        extreme_fcs$c,\n"
done
printf "        extreme_fcs$max_reps\n    };\n"

cat << EOM
    unsigned long  func_index = replicates - 2;
    
    return extreme_fcs_funcs[func_index](count_pairs, pair_count,
				   observed_fc, fc_count);
}
EOM
