/*********************************************
Author: Mohamad Mansouri
Email: Mohamad.Mansouri@eurecom.fr
Date: 11/09/2018
**********************************************/

#ifndef _FLOAT_MULT_
#define _FLOAT_MULT_

#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/parse_options.h>
#include <abycore/aby/abyparty.h>
#include <abycore/circuit/share.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/circuit/arithmeticcircuits.h>

#include <abycore/sharing/sharing.h>
#include <cassert>
#include <iomanip>
#include <iostream>

using namespace std;
// Structure for holding the max value and index
struct predicted
{
	share* max;
	share* max_ind;
};


// Method to extract an indexed value from a SIMD arithmetic share as a new arithmetic share
share* extract_index(share *s_x , uint32_t i);

// Method that takes 2 SIMD boolean shares (Repeated Vector & Flattened Matrix) ,does the Inner product and then sum the values to return a vector-matrix dot operation 
share* InnerProduct (share* s_input, share* s_weight, share* s_bias, uint32_t rows,uint32_t cols, uint32_t n_sig);

// Method that takes a SIMD arithmetic share and returns the Max value and index as a share
share* find_max(share* s_y, uint32_t nvals, uint32_t n_sig);

// Main Method that does the prediction function ( this Method is called by the 2 parties specifying their roles)
vector<char> predict(e_role role, const string& address, uint16_t port, seclvl seclvl, uint32_t nthreads,
	e_mt_gen_alg mt_alg, vector<vector<double>> input, vector<vector<double>> weight_all[2],
	 vector<double> bias_all[2], uint32_t n_sig
#ifdef PCA
	 ,vector<vector<double>> pca_all, vector<double> mean_all 
#endif
	 );

#endif