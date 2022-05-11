/***************************************************************************
 *  Description:
 *      Compute normalized abundances from raw counts provided in a TSV file
 *      similar to Kallisto abundances.tsv.  The TSV can be generated by
 *      the abundance subcommand from alignment data if necessary.
 *
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2022-05-04  Jason Bacon Begin
 ***************************************************************************/

#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>e
#include <xtend/dsv.h>
#include "normalize.h"

int     main(int argc,char *argv[])

{
    dsv_line_t  dsv_line;
    // One abundance file for each sample
    // char *abundance_files[MAX_SAMPLES]
    // FILE *abundance_stream[MAX_SAMPLES]
    // Array of ratios for each sample
    // double *ratios[MAX_SAMPLES];
    
    switch(argc)
    {
	case 1:
	    break;
	
	default:
	    usage(argv);
    }

    /*
     *  Median of ratios normalization
     *  https://scienceparkstudygroup.github.io/research-data-management-lesson/median_of_ratios_manual_normalization/index.html
     *  Similar to TMM but more robust: doi 10.1093/bib/bbx008
     *
     *  First sweep:
     *
     *  Read raw counts for all genes and all samples
     *
     *  1.  Take log of every count (just for filtering in step 3?)
     *  2.  Average of all samples for the gene (compute pseudo-reference)
     *  3.  Remove genes witn -inf as average
     *  4.  Subtract pseudo-reference from each log(expression)
     *      This is actually a ratio since subtracting a log is dividing
     *      We'll need to store this value and later sort to find median
     *
     *  After first sweep:
     *
     *  5.  Take the median of the ratios for each sample
     *  6.  exp(median) = count scaling factor
     *  7.  Divide counts by scaling factor
     */
    
    dsv_line_init(&dsv_line);
    // Abundance file format:
    // target_id       length  eff_length      est_counts      tpm
    // for (sample = 0; sample < sample_count; ++sample)
    // {
    //      Read one gene from each sample.  EOF should arrive on all together.
    // }
    while ( dsv_line_read(&dsv_line, stdin, "\t") != EOF )
    {
	// Dummy output: Just echo non-normalized counts to test UI
	printf("%s\t%s\n", DSV_LINE_FIELDS_AE(&dsv_line, 0),
		DSV_LINE_FIELDS_AE(&dsv_line, 3));
    }
    return EX_OK;
}


void    usage(char *argv[])

{
    fprintf(stderr, "Usage: %s\n", argv[0]);
    exit(EX_USAGE);
}
