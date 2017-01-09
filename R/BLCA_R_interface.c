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

	Last modification of this code: Mon 09 May 2016 16:27:18 IST  			
*/

#include "BLCA_mixmod.h"

void BLCA_VS(int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *fixed_groups, int *just_gibbs_updates, int *init_n_groups, int *max_n_groups, int *n_iterations, int *n_burn_in, int *thin_by, int *n_gibbs, int *group_memberships, int *n_groups, int *prior_G, int *variable_select, int *variable_inclusion_indicator, double *prior_prob_include, double *log_joint_posterior, int *hprior_model, double *prior_include, int *var_pattern );

void BLCA_VS_COMPUTE_POST_HOC_PARAMETER_ESTIMATES(int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *n_groups, int *n_sample, int *memberships, int *variable_inclusion_indicator, int *variable, double *estimate, double *sd_estimate, double *classprob_estimate, double *sd_classprob_estimate ) ; 

void BLCA_GIBBS_SAMPLER(int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *n_groups, int *n_iterations, int *n_burn_in, int *thin_by, int *group_memberships, double *group_weights, double *prob_variables, int *var_in, double *log_joint_posterior, int *verbose, int *verbose_update );

void BLCA_EM_FIT( int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *n_groups, int *max_iterations, int *iterations, double *group_probabilities, double *group_weights, double *prob_variables, int *var_in, double *log_likelihood, int *MAP, double *tol );

void BLCA_RELABEL( int *n_obs, int *n_sample, int *n_groups, int *labels_in, int *labels_out, int *permutation );

void BLCA_VS(int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *fixed_groups, int *just_gibbs_updates, int *init_n_groups, int *max_n_groups, int *n_iterations, int *n_burn_in, int *thin_by, int *n_gibbs, int *group_memberships, int *n_groups, int *prior_G, int *variable_select, int *variable_inclusion_indicator, double *prior_prob_include, double *log_joint_posterior, int *hprior_model, double *prior_include, int *var_pattern )
{

	int i,j,n,d,inG,mxG,nit,nburn,fixed,justgibbs;
	struct mix_mod *mixmod;
	struct results *results;
	
	mixmod = BLCA_allocate_mixmod( *nobs, *nvar, *max_n_groups, *init_n_groups, hparam, ncat, TRUE, FALSE );
	
	if(*prior_G == 0){
		BLCA_set_prior_on_number_of_components(mixmod,RICHARDSON_AND_GREEN);
	}else{
		BLCA_set_prior_on_number_of_components(mixmod,NOBILE_AND_FEARNSIDE);
	}	
	
	int x;
	
	for(i=0;i<*nobs;i++){
		for(j=0;j<*nvar;j++){
			x = Y[i + j*(*nobs)];
			mixmod->Y[j][i] = x;
			mixmod->Yobs[i][j] = x;
		}
	}
	
	mixmod->n_gibbs = (*n_gibbs);
	
	/*set prior for variable inclusion-- say 50% variables relevant for explaining clustering*/
	
	mixmod->prior_prob_variable_include = (*prior_prob_include);
	
	/*should there be a hyperprior put on this value?*/
	
	mixmod->hprior_model = (*hprior_model);
	
	if(mixmod->hprior_model)
	{
		mixmod->hprior_model_a0 = 1.;
		mixmod->hprior_model_b0 = (double)mixmod->d/4.; //use d/4 as a default value- this favours more parsimony in clustering variables
	}

	BLCA_initialize_simple(mixmod,*init_n_groups);
	
	GetRNGstate();
	
	//this call does the MCMC
	
	results = BLCA_analysis_MCMC_collapsed(mixmod,*n_iterations,*n_burn_in,*thin_by,*fixed_groups,*just_gibbs_updates,*variable_select,
												group_memberships, variable_inclusion_indicator, n_groups, log_joint_posterior, prior_include, var_pattern );
												
	PutRNGstate();

	BLCA_free_results(results,*n_iterations,*n_burn_in,*thin_by);

	BLCA_free_mixmod(mixmod);
	
	return;

}




void BLCA_VS_COMPUTE_POST_HOC_PARAMETER_ESTIMATES(int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *n_groups, int *n_sample, int *memberships, int *variable_inclusion_indicator, int *variable, double *estimate, double *sd_estimate, double *classprob_estimate, double *sd_classprob_estimate )
{

	int i,j,n,d,inG;
	struct mix_mod *mixmod;
	struct results *input;
	double **Estimate,**SE_Estimate;	

	mixmod = BLCA_allocate_mixmod( *nobs, *nvar, *n_groups, *n_groups, hparam, ncat, TRUE, FALSE );
	
	//mixmod->prior_prob_variable_include = (*prior_prob_include);
	
	input = (struct results *)malloc(sizeof(struct results));
	BLCA_allocate_results_x2(input,*n_sample,0,1,*nobs,*nvar,FALSE);

	int x;
	
	for(i=0;i<*nobs;i++){
		for(j=0;j<*nvar;j++){
			x = Y[i + j*(*nobs)];
			mixmod->Y[j][i] = x;
			mixmod->Yobs[i][j] = x;
		}
	}	
		
	for(i=0;i<*n_sample;i++){
		for(j=0;j<*nobs;j++){
			input->memberships[i][j] = memberships[ j*(*n_sample) + i ];
		}
		for(j=0;j<*nvar;j++){
			input->variable_indicator[i][j] = variable_inclusion_indicator[ j*(*n_sample) + i ];
		}
	}
	
	/*allocate the space to store estimated parameters*/
	
	Estimate = calloc(*n_groups,sizeof(double *));
	SE_Estimate = calloc(*n_groups,sizeof(double *));
	for(i=0;i<*n_groups;i++){
		Estimate[i] = calloc(mixmod->ncat[*variable],sizeof(double));
		SE_Estimate[i] = calloc(mixmod->ncat[*variable],sizeof(double));
	}

	BLCA_compute_post_hoc_parameter_estimates_for_variable( mixmod, input, *n_sample, *n_groups, *variable, Estimate, SE_Estimate );

	
	for(i=0;i<*n_groups;i++){
		for(j=0;j<mixmod->ncat[*variable];j++){
			//Rprintf("\nThe value of i = %d, the value of j = %d",i,j);
			estimate[j*(*n_groups) + i] = Estimate[i][j];
			sd_estimate[j*(*n_groups) + i] = SE_Estimate[i][j];
		}
	}
	
	BLCA_compute_post_hoc_parameter_estimates_for_class_probabilities( mixmod, input, *n_sample, *n_groups, 0, classprob_estimate, sd_classprob_estimate );


	for(i=0;i<*n_groups;i++){
		free(Estimate[i]);
		free(SE_Estimate[i]);
	}
	free(Estimate);
	free(SE_Estimate);
	
	BLCA_free_results_x2(input,*n_sample,0,1,FALSE);
	BLCA_free_mixmod(mixmod);

	return;

}

/*------------------------------------original sampler not collapsed-------------------------------*/

void BLCA_GIBBS_SAMPLER(int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *n_groups, int *n_iterations, int *n_burn_in, int *thin_by, int *group_memberships, double *group_weights, double *prob_variables, int *var_in, double *log_joint_posterior, int *verbose, int *verbose_update )
{

	int i,j,n,d,inG,mxG,nit,nburn,fixed,justgibbs;
	struct mix_mod *mixmod;
	
	mixmod = BLCA_allocate_mixmod( *nobs, *nvar, *n_groups, *n_groups, hparam, ncat, FALSE, FALSE );

	for(j=0;j<mixmod->d;j++){
		mixmod->varindicator[j] = var_in[j];
	}
	
	int x;
	
	for(i=0;i<*nobs;i++){
		for(j=0;j<*nvar;j++){
			x = Y[i + j*(*nobs)];
			mixmod->Y[j][i] = x;
			mixmod->Yobs[i][j] = x;
		}
	}
	
	//important... need a function to initialize the algorithm...a
	BLCA_initialize_simple(mixmod,*n_groups);
	
	//for(j=0;j<*nvar;j++) mixmod->varindicator[j] = 1;
	
	GetRNGstate();
	
	BLCA_analysis_MCMC_Gibbs_sampler(mixmod,*n_iterations,*n_burn_in,*thin_by,
												group_memberships, group_weights, prob_variables, log_joint_posterior, *verbose, *verbose_update );
	
	PutRNGstate();

	BLCA_free_mixmod(mixmod);
		
	return;
}

/*-----------------------------------------fit using EM algorithm----------------------------------------*/

void BLCA_EM_FIT( int *Y, int *nobs, int *nvar, int *ncat, double *hparam, int *n_groups, int *max_iterations, int *iterations, double *group_probabilities, double *group_weights, double *prob_variables, int *var_in, double *log_likelihood, int *MAP, double *tol )
{

		struct mix_mod *mixmod;
		int i, j;
		//if MAP == 1 then a prior regularizer is used in the EM through the hparam vector
		
		mixmod = BLCA_allocate_mixmod( *nobs, *nvar, *n_groups, *n_groups, hparam, ncat, FALSE, FALSE );
		
		for(j=0;j<mixmod->d;j++){
	  	 	mixmod->varindicator[j] = var_in[j];
		}
		
		//initialize by using the values in group_weights and prob_variables
		
		BLCA_initialize_EM( mixmod , group_weights, prob_variables);
		
		int x;
	
		for(i=0;i<*nobs;i++){
			for(j=0;j<*nvar;j++){
				x = Y[i + j*(*nobs)];
				mixmod->Y[j][i] = x;
				mixmod->Yobs[i][j] = x;
			}
		}			

		//important... need a function to initialize the algorithm...a
		//can most likely pass the  initialization in through the arg--
		//initialize_EM(mixmod,*n_groups);	
			
		
		BLCA_analysis_EM( mixmod, *max_iterations, *iterations, group_probabilities, group_weights, prob_variables, log_likelihood, *MAP, *tol ) ;
		
		free(mixmod);
		
		return;
}


//label switching algorithm

void BLCA_RELABEL( int *n_obs, int *n_sample, int *n_groups, int *labels_in, int *labels_out, int *permutation )
{

	int n,g,**raw,**relab,**summary,
		i,j,k,t,N,T,**cost,*lab;
	
	/*n and no. of groups*/
	n=n_obs[0];
	g=n_groups[0];
	/*N = number of samples to undo label switching for*/
	N=n_sample[0];

	T=0;

	/*allocate memory and initialize*/
	raw = imatrix(1,N,1,n);
	relab = imatrix(1,N,1,n);
	summary = imatrix(1,g,1,n);
	cost = imatrix(1,g,1,g+1);
	lab = ivector(1,g);


	/*copy from labels_in to raw_r*/
	for(i=0;i<N;i++){
		for(j=0;j<n;j++){
			raw[i+1][j+1] = labels_in[ i + j*N ];
		}	
	}


	for(i=1;i<g+1;i++){
		for(j=1;j<n+1;j++){
			summary[i][j]=0;
		}
	}

	/*use the first allocation in raw as the first labelling*/

	for(i=1;i<n+1;i++){
		summary[raw[1][i]][i]+=1;
	}

	for(i=1;i<n+1;i++){
		relab[1][i] = raw[1][i];
	}
	

	for(i=1;i<g+1;i++){
		permutation[(i-1)*N + 0] = i; //this is the identity mapping as nothing done.
	}

	for(t=2;t<N+1;t++){

	/*row t*/
	/*compute the cost matrix*/
	for(i=1;i<g+1;i++){
	
		for(j=1;j<g+1;j++){
	
			cost[i][j]=0;
		
			for(k=1;k<n+1;k++){
		
				if(raw[t][k]==j){
					cost[i][j]+=summary[i][k];
				}
			
			} 
		
			cost[i][j]=n*(t-1)-cost[i][j];
		
		}
		cost[i][g+1]=0;
	
		}

		T=0;
		
		assct(g,cost,lab,&T);

		/*store the permutation*/
		for(i=1;i<g+1;i++){
			permutation[ (i-1)*N + (t-1) ] = lab[i];
		}

		/*relabel based on output from assct*/
		/*update the summary matrix*/
		for(i=1;i<n+1;i++){
			relab[t][i]=lab[raw[t][i]];
			summary[lab[raw[t][i]]][i]+=1;
		}

	}


	for(i=0;i<N;i++){
		for(j=0;j<n;j++){
			labels_out[ i + j*N ] = relab[i+1][j+1];
		}	
	}	
	
	free_imatrix(raw,1,N,1,n);
	free_imatrix(relab,1,N,1,n);
	free_imatrix(summary,1,g,1,n);
	free_imatrix(cost,1,g,1,g+1);
	free_ivector(lab,1,g);

	return;
}



