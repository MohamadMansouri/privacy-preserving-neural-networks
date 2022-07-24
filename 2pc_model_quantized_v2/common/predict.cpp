/*********************************************
Author: Mohamad Mansouri
Email: Mohamad.Mansouri@eurecom.fr
Date: 11/09/2018
**********************************************/

#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/parse_options.h>
#include <abycore/aby/abyparty.h>
#include <abycore/circuit/share.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/circuit/arithmeticcircuits.h>
#include <math.h>

#include <abycore/sharing/sharing.h>
#include <cassert>
#include <iomanip>
#include <iostream>
#include "predict.h"

using namespace std;

const char classes[16] = {'N', 'A','V','/','f','+','x','F','j','L','a','J','R','!','E','e'};

// build circuits
static ArithmeticCircuit* ac;
#ifdef MAX
static BooleanCircuit* bc;
static BooleanCircuit* yc;
#endif

// we operate on doubles, so set bitlen to 64 bits
const static uint32_t bitlen = 64;


// The number of neurons in the input layer
const static uint32_t nvals_i = 16;
// The number of neurons in the hidden layer
const static uint32_t nvals_h = 38;
// The number of neurons in the output layer
const static uint32_t nvals_o = 16;

// The dimension of the signal before PCA
#ifdef PCA
const static uint32_t nvals_s = 180;
const static uint32_t prec_i = 2;
#else
const static uint32_t nvals_s = nvals_i;
const static uint32_t prec_i = 3;
#endif



// retrun an element of SIMD at certain index
share* extract_index(share *s_x , uint32_t i) 
{
	uint64_t zero = 0;
	share* s_dummy = ac->PutCONSGate(zero,bitlen);

	s_dummy->set_wire_id(0, s_x->get_wire_id(i));

	return s_dummy;
}



void convert_vec_to_int(uint64_t arr[], vector<double> vec, uint32_t nvals, int prec)
{	
	for(uint32_t i=0;i<nvals;i++)
		arr[i] = (uint64_t) round(vec[i]*pow(10,prec));		
}



void convert_mat_to_int(uint64_t arr[], vector<vector<double>> vec, uint32_t rows, uint32_t cols, int prec)
{
	for (uint32_t j=0;j<cols;j++)
		for(uint32_t i=0;i<rows;i++)
			arr[i + rows * j] = (uint64_t) round(vec[i][j] * pow(10,prec));
}



#ifdef MAX
share* find_max(share* s_y, uint32_t nvals)
{
	uint64_t ngtv_zero = 0x8000000000000000;
	uint64_t zero = 0x0000000000000000;
	struct predicted s_result;
	share* s_zero = bc->PutCONSGate(zero, bitlen);
	share* s_ngtv_zero = bc->PutCONSGate(ngtv_zero, bitlen);

	s_result.max = extract_index(s_y,0);
	s_result.max = bc->PutA2BGate(s_result.max,yc);
	s_result.max = bc->PutMUXGate(s_zero,s_result.max,bc->PutGTGate(s_result.max, s_ngtv_zero));
	s_result.max_ind = ac->PutCONSGate(zero,bitlen); 
	s_result.max_ind = bc->PutA2BGate(s_result.max_ind,yc);


	share* tmp;
	for (uint32_t i = 1; i < nvals; i++) 
	{
		uint64_t index = i;
		tmp = extract_index(s_y,i);
		tmp = bc->PutA2BGate(tmp,yc);
		tmp = bc->PutMUXGate(s_zero,tmp,bc->PutGTGate(tmp, s_ngtv_zero));
		s_result.max = bc->PutMUXGate(s_result.max,tmp,bc->PutGTGate(s_result.max,tmp));
		s_result.max_ind = bc->PutMUXGate(s_result.max_ind,bc->PutCONSGate(index,bitlen),bc->PutGTGate(s_result.max,tmp));


	}
	return s_result.max_ind;


}
#endif


share* InnerProduct (share* s_input, share* s_weight, share* s_bias, uint32_t rows,uint32_t cols)
{

	// Multiplication gate
 	share* s_mult =  ac->PutMULGate(s_input,s_weight);
	
	// Sum
	uint32_t pos_id[cols];
	// uint32_t pos_id_h[38];

	int j =0;
	for (uint32_t i=0; i< cols*rows; i+=rows)
		pos_id[j++]=i;

	share* s_sum = ac->PutSubsetGate(s_mult,pos_id,cols);
	share* tmp;
	
	for (uint32_t i = 1; i < rows; i++) 
	{
		
		for (uint32_t j = 0; j < cols; j++)
			pos_id[j]+=1; 

		tmp = ac->PutSubsetGate(s_mult,pos_id,cols);
		s_sum = ac->PutADDGate(s_sum,tmp);

	}
	if(s_bias)
		s_sum = ac->PutADDGate(s_sum,s_bias);

	return s_sum;

}

share* replicate(share* input, uint32_t nvals)
{

	uint32_t i = nvals;
	share* tmp = NULL;

	while (i > 1)
	{
		if (i%2 == 0)
		{
			input = ac->PutCombinerGate(input,input);
			i /= 2;
		}
		else
		{
			if (!tmp)
				tmp = input;
			else
				tmp = ac->PutCombinerGate(input,tmp);
			input = ac->PutCombinerGate(input,input);
			i = (i - 1) / 2;
		}
	}
	if(tmp)
		input = ac->PutCombinerGate(input,tmp);

	return input;
}



void predict(e_role role, const string& address, uint16_t port, seclvl seclvl, uint32_t nthreads,
	e_mt_gen_alg mt_alg, vector<double> input, vector<vector<double>> weight_all[2],
	vector<double> bias_all[2]
#ifdef PCA
	 ,vector<vector<double>> pca_all, vector<double> mean_all 
#endif
	 )
{

	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads, mt_alg, 12000000);

	vector<Sharing*>& sharings = party->GetSharings();

	ac = (ArithmeticCircuit*) sharings[S_ARITH]->GetCircuitBuildRoutine();
	#ifdef MAX
	bc = (BooleanCircuit*) sharings[S_BOOL]->GetCircuitBuildRoutine();
	yc = (BooleanCircuit*) sharings[S_YAO]->GetCircuitBuildRoutine();
	#endif


	uint64_t input_arr[nvals_s];
	uint64_t weight_arr_h[nvals_i*nvals_h];
	uint64_t bias_arr_h[nvals_h];

#ifdef PCA
	uint64_t pca_arr[nvals_s*nvals_i];
	uint64_t mean_arr[nvals_s];
#endif

	// Fill the inputs of each role to the arrays
	if (role == CLIENT)
		convert_vec_to_int(input_arr,input,nvals_s,prec_i);

	if (role == SERVER)
		{
			convert_vec_to_int(bias_arr_h,bias_all[0],nvals_h,6);
			convert_mat_to_int(weight_arr_h, weight_all[0],nvals_i,nvals_h,prec_i);
#ifdef PCA			
			convert_vec_to_int(mean_arr,mean_all,nvals_s,prec_i);
			convert_mat_to_int(pca_arr, pca_all,nvals_s,nvals_i,prec_i);
#endif
		}	

	share* s_input = ac->PutSIMDINGate(nvals_s, input_arr, bitlen, CLIENT);

#ifdef PCA
	cout << "[-] Building PCA gates..." << endl;

	share* s_mean = ac->PutSIMDINGate(nvals_s, mean_arr, bitlen, SERVER);

	s_input = ac->PutADDGate(s_input,s_mean);

	// Replicating input 
	s_input = replicate(s_input, nvals_i);
	
	share* s_pca = ac->PutSIMDINGate(nvals_s*nvals_i, pca_arr, bitlen, SERVER);
	
	s_input = InnerProduct(s_input, s_pca, NULL, nvals_s, nvals_i);

#endif

	cout << "[-] Building Hidden Layer Circuit..." << endl;
	
	// Replicating input 
	s_input = replicate(s_input, nvals_h);

	// Yh= I.Wh + Bh

	share* s_bias = ac->PutSIMDINGate(nvals_h,bias_arr_h,bitlen,SERVER);
	share* s_weight = ac->PutSIMDINGate(nvals_i*nvals_h, weight_arr_h, bitlen, SERVER);
	
	share* s_yh= InnerProduct(s_input, s_weight, s_bias, nvals_i,nvals_h);

	// activation function (x2)
	s_yh = ac->PutMULGate(s_yh,s_yh);

	cout << "[-] Building Output Layer..." << endl;

	// Replicating yh 
	s_yh = replicate(s_yh, nvals_o);
	
	// Y = Yh.Wo + Bo
	
	uint64_t bias_arr_o[nvals_o];
	uint64_t weight_arr_o[nvals_h*nvals_o];
	

	if (role == SERVER)
	{
		convert_vec_to_int(bias_arr_o,bias_all[1],nvals_o,15);
		convert_mat_to_int(weight_arr_o, weight_all[1],nvals_h,nvals_o,3);
	}



	s_bias = ac->PutSIMDINGate(nvals_o,bias_arr_o,bitlen,SERVER);
	s_weight = ac->PutSIMDINGate(nvals_h*nvals_o, weight_arr_o, bitlen, SERVER);



	share* s_y = InnerProduct(s_yh, s_weight,s_bias, nvals_h, nvals_o);

#ifdef MAX

	cout << "[-] Building SoftMax Layer......" << endl;

	s_y = ac->PutSplitterGate(s_y);

	share* s_argmax = find_max(s_y, nvals_o);


	s_argmax = bc->PutOUTGate(s_argmax,CLIENT);

#else

	s_y = ac->PutOUTGate(s_y,CLIENT);

#endif
	
	cout << "[-] Start circuit execution..." << endl;

	party->ExecCircuit();



	// retrieve plain text output
	if (role == CLIENT)
	{		
#ifdef MAX		

		uint64_t max_ind = s_argmax->get_clear_value<uint64_t>();
		cout << "Class: "<< classes[*(int*) &max_ind]<< endl;

#else

		uint32_t out_bitlen, out_nvals;
		uint64_t *out_vals;

		s_y->get_clear_value_vec(&out_vals, &out_bitlen, &out_nvals);
		for (uint32_t i = 0; i < out_nvals; i++) 
		{
			double val = ((signed long int) out_vals[i]);
			cout << classes[i] << ": " << val/pow(10,15) << endl;
		}
		

#endif

	}


 }