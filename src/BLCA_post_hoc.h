/*Functions for the fitting of Latent Class Analysis models using MCMC
	methods. Two implementations of the model are included in the Bayesian
	formulation: collapsed and not collapsed.
	
	Author:	Jason Wyse,
			School of Computer Science and Statistics,
			Lloyd Institute,
			Trinity College,
			Dublin 2,
			Ireland.
			mailto: wyseja@tcd.ie
			
	Last modification of this code: Wed 11 May 2016 04:03:05 IST    */

#ifndef __BLCA_POST_HOC_H__
#define __BLCA_POST_HOC_H__

#include "BLCA_mixmod.h"

void BLCA_compute_post_hoc_parameter_estimates_for_variable( struct mix_mod *mixmod, struct results *input, int n_sample, int n_groups, int variable, double **Estimate, double **SE_Estimate ) ;

void BLCA_compute_post_hoc_parameter_estimates_for_class_probabilities( struct mix_mod *mixmod, struct results *input, int n_sample, int n_groups, int variable, double *Estimate, double *SE_Estimate) ;

#endif
