/***************************************************************************
 *  Description:
 *      Test driver for computing exact P-values of differential
 *      expression data.
 *
 *  History: 
 *  Date        Name        Modification
 *  2022-09-24  Jason Bacon Begin
 ***************************************************************************/

#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>
#include <xtend/math.h>
#include <math.h>
#include <time.h>
#include <limits.h>

unsigned long   fc_ge_count(double fc_list[], unsigned long fc_count,
		      unsigned long replicates, double observed_fc_mean,
		      unsigned long *fc_mean_count);
void    fc_mean_exact_p_val(double fc_list[], size_t fc_count,
			    size_t replicates,
			    double observed_fc_mean, double observed_fc_stddev,
			    double observed_fc_spread, double dist_fc_mean);

void    usage(char *argv[])

{
    fprintf(stderr, "Usage:   %s count1-mean count2-mean max-deviation\n", argv[0]);
    fprintf(stderr, "Example: %s 100 200 .2 3\n",argv[0]);
    exit(EX_USAGE);
}

int     main(int argc,char *argv[])

{
    unsigned long   c, c1, c2, replicates,
		    less, more, equal, *counts,
		    count1_mean, count2_mean, temp, fc_count, samples, fc_ge,
		    half_fc_count;
    double  fc, observed_fc_mean, *fc_list, max_deviation, fc_sum,
	    dist_fc_mean, fc_var_sum, observed_fc_stddev,
	    observed_fc_min, observed_fc_max, observed_fc_spread;

    if ( argc != 5 )
	usage(argv);

    count1_mean = atoi(argv[1]);
    count2_mean = atoi(argv[2]);
    max_deviation = atof(argv[3]);
    replicates = atoi(argv[4]);
    samples = replicates * 2;
    
    counts = malloc(samples * sizeof(*counts));
    
    // Some code below assumes count2_mean > count1_mean
    if ( count1_mean > count2_mean )
    {
	temp = count1_mean;
	count1_mean = count2_mean;
	count2_mean = temp;
    }
    
    /*
     *  Generate samples random counts with FC around count2 / count1
     *  These are the "observed" counts
     */
    printf("\ncount1 = %lu +/- to up to %0.0f%%, count2 = %lu +/- same\n",
	    count1_mean, max_deviation * 100, count2_mean);
    puts("Cond1 Cond2");
    
    // Comment this out to get the same counts repeatedly
    srandom(time(NULL));
    observed_fc_min = 1000000000;
    observed_fc_max = 0;
    for (c = 0, observed_fc_mean = 0.0; c < replicates; ++c)
    {
	counts[c] = count1_mean
		 + random() % (int)(count1_mean * max_deviation * 2)
		 - count1_mean * max_deviation;
	counts[c + replicates] = count2_mean
		 + random() % (int)(count2_mean * max_deviation * 2)
		 - count2_mean * max_deviation;
	fc = (double)counts[c + replicates] / counts[c];
	printf("%5lu %5lu %0.5f\n", counts[c], counts[c + replicates], fc);
	observed_fc_mean += fc;
	if ( fc > observed_fc_max ) observed_fc_max = fc;
	if ( fc < observed_fc_min ) observed_fc_min = fc;
    }
    observed_fc_spread = observed_fc_max - observed_fc_min;
    
    /*
     *  Compute mean and stddev for "observed" values
     */
    
    observed_fc_mean /= replicates;
    fc_var_sum = 0;
    for (c = 0; c < replicates; ++c)
    {
	fc = (double)counts[c + replicates] / counts[c];
	fc_var_sum += (fc - observed_fc_mean) * (fc - observed_fc_mean);
    }
    observed_fc_stddev = sqrt(fc_var_sum / replicates);
    printf("Observed FC mean = %0.5f\n", observed_fc_mean);
    printf("P-value: Likelihood of a mean of %lu FCs at least as extreme as %0.5f\n",
	    replicates, observed_fc_mean);
    
    /*
     *  Compute fold-change for every possible pairing.
     *  #samples choose 2 * 2 (FC and 1/FC), since this affects
     *  the distribution of means of N samples
     *  Satisfies the null hypothesis P(n1 > n2) = P(n1 < N2)
     */
    
    printf("\n%lu choose %d = %lu combinations of counts\n",
	    samples, 2, xt_n_choose_k(samples, 2));
    puts("2 fold-changes for each combination of samples:");
    half_fc_count = xt_n_choose_k(samples, 2);
    fc_count = half_fc_count * 2;   // FC and 1/FC
    fc_list = malloc(fc_count * sizeof(*fc_list));
    
    /*
     *  Include both FC and 1/FC for each count combination in the set
     *  so that the distribution of FC means covers both cases.
     */
    
    c = fc_sum = fc_ge = less = more = equal = 0;
    for (c1 = 0; c1 < replicates * 2; ++c1)
    {
	for (c2 = c1 + 1; c2 < replicates * 2; ++c2)
	{
	    fc_list[c] = (double)counts[c1] / counts[c2];
	    fc_list[c + half_fc_count] = 1.0 / fc_list[c];
	    printf("%2lu %3lu / %3lu = %0.5f\n", c,
		    counts[c1], counts[c2], fc_list[c]);
	    printf("%2lu %3lu / %3lu = %0.5f\n", c + half_fc_count,
		    counts[c2], counts[c1], fc_list[c + half_fc_count]);
	    if ( fc_list[c] >= observed_fc_mean )
		++fc_ge;
	    
	    if ( fc_list[c] < 1.0 )
		++less, ++more;
	    else if ( fc_list[c] > 1.0 )
		++more, ++less;
	    else
		equal += 2;
	    fc_sum += fc_list[c] + fc_list[c + half_fc_count];
	    ++c;
	}
    }
    
    // Check for program bugs
    if ( c != half_fc_count )
    {
	printf("%lu != %lu\n", c, half_fc_count);
	return 1;
    }
    
    dist_fc_mean = fc_sum / fc_count;
    printf("\nLess + more + equal should be %lu.  FC mean should be > 1.\n",
	    fc_count);
    printf("Less should equal more (distribution is symmetric).\n");
    printf("Distribution: less = %lu  more = %lu  equal = %lu  H0 FC mean = %0.5f\n",
	    less, more, equal, dist_fc_mean);

    fc_mean_exact_p_val(fc_list, fc_count, replicates, observed_fc_mean,
			observed_fc_stddev, observed_fc_spread, dist_fc_mean);
    return EX_OK;
}


/*
 *  Find all possible means of fold-change triplets
 *  Count the number of means >= observed mean
 */

void    fc_mean_exact_p_val(double fc_list[], size_t fc_count,
			    size_t replicates, double observed_fc_mean,
			    double observed_fc_stddev,
			    double observed_fc_spread, double dist_fc_mean)

{
    unsigned long   fc_mean_count, fc_ge, c;

    if ( replicates <= 10 )
    {
	fc_mean_count = xt_n_choose_k(fc_count, replicates);
	printf("\n%zu choose %zu = %lu possible means of %lu FCs\n",
	    fc_count, replicates, fc_mean_count, replicates);
    }
    else
	printf("\nfc_mean_count > 2^64 for replicates > 10.\n");

    // Run 10 reps for down-sampled FC means to check stability
    for (c = 0; c < (replicates > 5 ? 5 : 1); ++c)
    {
	fc_ge = fc_ge_count(fc_list, fc_count, replicates,
			    observed_fc_mean, &fc_mean_count);
	printf("\nLower FC mean, higher stddev, and outlier counts cause higher P-values.\n");
	printf("Observed: FC mean = %0.5f  stddev = %0.5f  spread = %0.5f\n",
		observed_fc_mean, observed_fc_stddev, observed_fc_spread);
	printf("FC mean count = %lu  FC >= %0.5f = %lu  P(FC >= %0.5f) = %0.5f\n\n",
		fc_mean_count, observed_fc_mean, fc_ge, observed_fc_mean,
		(double)fc_ge / fc_mean_count);
    }
}
