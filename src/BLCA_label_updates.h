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
			
	Last modification of this code: Mon 09 May 2016 15:38:03 IST   */

#ifndef __BLCA_LABEL_UPDATES_H__
#define __BLCA_LABEL_UPDATES_H__

#include "BLCA_mixmod.h"

int BLCA_update_allocations_with_gibbs(struct mix_mod *mixmod) ;

double BLCA_get_log_relative_probability_for_gibbs(int *x,struct component *component_k,struct component *component_g,struct mix_mod *mixmod) ;

int BLCA_update_allocations_with_metropolis_move_1(struct mix_mod *mixmod,int *accepted,int *proposed) ;

int BLCA_update_allocations_with_metropolis_move_2(struct mix_mod *mixmod,int *accepted,int *proposed) ;

int BLCA_update_allocations_with_metropolis_move_3(struct mix_mod *mixmod,int *accepted,int *proposed) ;

#endif
