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

// Structure for holding the max value and index
struct predicted
{
	share* max;
	share* max_ind;
};


// Method to extract an indexed value from a SIMD arithmetic share as a new arithmetic share
share* extract_index(share *s_x , uint32_t i);


// Method that takes 2 SIMD boolean shares ,does the Inner product and then sum the values to return a vector-matrix dot operation 
share* InnerProduct (share* s_input, share* s_weight,uint32_t nvals);

// Method to convert the input signal from a vector of type double to array of type uint64
void convert_arr_to_int(uint64_t arr[], std::vector<double> vec, uint32_t nvals);

// Method to convert the weights from a double vector of type double to array of type uint64
void convert_mat_to_int(uint64_t arr[], std::vector<std::vector<double>> vec, uint32_t nvals,uint32_t j);
 
// Method that takes a SIMD arithmetic share and returns the Max value and index as a share
struct predicted find_max(share* s_y, uint32_t nvals);

// Main Method that does the prediction function ( this Method is called by the 2 parties specifying their roles)
void predict(e_role role, const std::string& address, uint16_t port, seclvl seclvl, uint32_t nthreads,
	e_mt_gen_alg mt_alg, std::vector<double> input, std::vector<std::vector<double>> weight[2], std::vector<double> bias[2]);


#endif