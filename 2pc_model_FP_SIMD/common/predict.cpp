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

// build circuits
static ArithmeticCircuit* ac;
static BooleanCircuit* bc;
static Circuit* yc;

// we operate on doubles, so set bitlen to 64 bits
const static uint32_t bitlen = 64;


// The number of neurons in the input layer
const static uint32_t nvals_i = 16;
// The number of neurons in the hidden layer
const static uint32_t nvals_h = 38;
// The number of neurons in the output layer
const static uint32_t nvals_o = 16;


char classes[16] = {'N', 'A','V','/','f','+','x','F','j','L','a','J','R','!','E','e'};


// retrun an element of SIMD at certain index
share* extract_index(share *s_x , uint32_t i) 
{
	uint64_t zero = 0;
	share* s_dummy = ac->PutCONSGate(zero,bitlen);

	s_dummy->set_wire_id(0, s_x->get_wire_id(i));

	return s_dummy;
}



void convert_vec_to_int(uint64_t arr[], std::vector<double> vec, uint32_t nvals)
{
	uint64_t *a;
	
	for(uint32_t i=0;i<nvals;i++)
	{
		a = (uint64_t*) &vec[i];
		arr[i] = *a;
	}
}
void convert_mat_to_int(uint64_t arr[], std::vector<std::vector<double>> vec, uint32_t rows, uint32_t cols)
{
	uint64_t *a;
	for (uint32_t j=0;j<cols;j++)
		for(uint32_t i=0;i<rows;i++)
		{
			a = (uint64_t*) &vec[i][j];
			arr[i+rows*j] = *a;
		}
}



struct predicted find_max(share* s_y, uint32_t nvals)
{
	int zero_int =0;
	uint64_t zero = *(uint64_t*) & zero_int;
	struct predicted s_result;
	s_result.max = extract_index(s_y,0);
	s_result.max = bc->PutY2BGate(yc->PutA2YGate(s_result.max));
	s_result.max_ind = bc->PutCONSGate(zero,bitlen); 
	share* tmp;
	for (uint32_t i = 1; i < nvals; i++) 
	{
		uint64_t index = i;
		tmp = extract_index(s_y,i);
		tmp = bc->PutY2BGate(yc->PutA2YGate(tmp));
		s_result.max = bc->PutMUXGate(s_result.max,tmp,bc->PutFPGate(s_result.max,tmp,CMP,bitlen,no_status));
		s_result.max_ind = bc->PutMUXGate(s_result.max_ind,bc->PutCONSGate(index,bitlen),bc->PutFPGate(s_result.max,tmp,CMP,bitlen,no_status));
	}	
	s_result.max = ac->PutB2AGate(s_result.max);
	s_result.max_ind = ac->PutB2AGate(s_result.max_ind);
	return s_result;

}



share* InnerProduct (share* s_input, share* s_weight, share* s_bias, uint32_t rows,uint32_t cols)
{
	// FP Multiplication gate
	share* s_mult = bc->PutFPGate(s_input, s_weight, MUL, bitlen, rows * cols + 1, no_status);
 
	//sum
	s_mult = ac->PutB2AGate(s_mult);


	uint32_t pos_id[cols];

	int j =0;
	for (uint32_t i=0; i< cols*rows; i+=rows)
		pos_id[j++]=i;

	share* s_sum = ac->PutSubsetGate(s_mult,pos_id,cols);
	s_sum = bc->PutA2BGate(s_sum,yc);
	share* tmp;
	for (uint32_t i = 1; i < rows; i++) 
	{
		for (uint32_t j = 0; j < cols; j++)
			pos_id[j]+=1; 

		tmp = ac->PutSubsetGate(s_mult,pos_id,cols);

		tmp = bc->PutA2BGate(tmp,yc);
		s_sum = bc->PutFPGate(s_sum,tmp,ADD,bitlen, cols,no_status);
	}
	s_sum = bc->PutFPGate(s_sum,s_bias,ADD,bitlen, cols,no_status);
	return s_sum;

}




void predict(e_role role, const std::string& address, uint16_t port, seclvl seclvl, uint32_t nthreads,
	e_mt_gen_alg mt_alg, std::vector<double> input, std::vector<std::vector<double>> weight_all[2], std::vector<double> bias_all[2]) {

	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads, mt_alg, 12000000);

	std::vector<Sharing*>& sharings = party->GetSharings();

	bc = (BooleanCircuit*) sharings[S_BOOL]->GetCircuitBuildRoutine();
	ac = (ArithmeticCircuit*) sharings[S_ARITH]->GetCircuitBuildRoutine();
	yc = (Circuit*) sharings[S_YAO]->GetCircuitBuildRoutine();


	uint64_t input_arr[nvals_i];
	uint64_t weight_arr_h[nvals_i*nvals_h];
	uint64_t bias_arr_h[nvals_h];


	// Fill the inputs of each role to the arrays
	if (role == CLIENT)
		convert_vec_to_int(input_arr,input,nvals_i);
	if (role == SERVER)
		{
			convert_vec_to_int(bias_arr_h,bias_all[0],nvals_h);
			convert_mat_to_int(weight_arr_h, weight_all[0],nvals_i,nvals_h);
		}	




	std::cout << "[-] Started Prediction..." << std::endl;


	share* s_input = ac->PutSIMDINGate(nvals_i, input_arr, bitlen, CLIENT);
	share* s_bias = bc->PutSIMDINGate(nvals_h,bias_arr_h,bitlen,SERVER);
	share* s_weight = bc->PutSIMDINGate(nvals_i*nvals_h, weight_arr_h, bitlen, SERVER);
	
	// Replicating input 

	uint32_t i = nvals_h;
	share* tmp = NULL;

	while (i > 1)
	{
		if (i%2 == 0)
		{
			s_input = ac->PutCombinerGate(s_input,s_input);
			i /= 2;
		}
		else
		{
			if (!tmp)
				tmp = s_input;
			else
				tmp = ac->PutCombinerGate(s_input,tmp);
			s_input = ac->PutCombinerGate(s_input,s_input);
			i = (i - 1) / 2;
		}
	}
	if(tmp)
		s_input = ac->PutCombinerGate(s_input,tmp);

	// Yh= (I.Wh + Bh).(I.Wh + Bh)

	s_input = bc->PutA2BGate(s_input,yc);

	share* s_yh= InnerProduct(s_input, s_weight, s_bias, nvals_i,nvals_h);


	// activation function (x2)

	s_yh = bc->PutFPGate(s_yh,s_yh,MUL,bitlen,nvals_h,no_status);

	
	std::cout << "[-] Finished Building Hidden Layer Circuit..." << std::endl;


	// Y = Yh.Wo + Bo

	s_yh = ac->PutB2AGate(s_yh);

	// Replicating s_yh
	i = nvals_o;
	tmp = NULL;

	while (i > 1)
	{
		if (i%2 == 0)
		{
			s_yh = ac->PutCombinerGate(s_yh,s_yh);
			i /= 2;
		}
		else
		{
			if (!tmp)
				tmp = s_yh;
			else
				tmp = ac->PutCombinerGate(s_yh,tmp);
			s_yh = ac->PutCombinerGate(s_yh,s_yh);
			i = (i - 1) / 2;
		}
	}
	if(tmp)
		s_yh = ac->PutCombinerGate(s_yh,tmp);	
	
	s_yh = bc->PutA2BGate(s_yh,yc);

	uint64_t bias_arr_o[nvals_o];
	uint64_t weight_arr_o[nvals_h];
	

	if (role == SERVER)
	{
		convert_vec_to_int(bias_arr_o,bias_all[1],nvals_o);
		convert_mat_to_int(weight_arr_o, weight_all[1],nvals_h,nvals_o);
	}


	s_bias = bc->PutSIMDINGate(nvals_o,bias_arr_o,bitlen,SERVER);
	s_weight = bc->PutSIMDINGate(nvals_h*nvals_o, weight_arr_o, bitlen, SERVER);


	share* s_y = InnerProduct(s_yh, s_weight,s_bias, nvals_h, nvals_o);

	
	s_y	= ac->PutB2AGate(s_y);		

	cout << "[-] Finished Output Layer Calculations..." << endl;



	// Max 
	s_y = ac->PutSplitterGate(s_y);
	struct predicted result = find_max(s_y,nvals_o);
	share* s_max_ind = result.max_ind;

	cout << "[-] Finished HardMax Calculations..." << endl;
	cout << endl << endl;



	// s_max = ac->PutOUTGate(s_max ,CLIENT);
	s_max_ind = ac->PutOUTGate(s_max_ind ,CLIENT);
	// s_y = ac->PutOUTGate(s_y,CLIENT);


	// run SMPC
	party->ExecCircuit();


	// retrieve plain text output
/*
	uint32_t out_bitlen, out_nvals;
	uint64_t *out_vals;
*/

	if (role == CLIENT)
	{
/*		s_y->get_clear_value_vec(&out_vals, &out_bitlen, &out_nvals);
		for (uint32_t i = 0; i < out_nvals; i++) {
			double val = *((double*) &out_vals[i]);
			cout << classes[i] << ": " << val << endl;
		}

		cout << endl << endl;

		uint64_t max = s_max->get_clear_value<uint64_t>();
		cout << *(double*) &max << endl;*/

		uint64_t max_ind = s_max_ind->get_clear_value<uint64_t>();
		cout << classes[*(int*) &max_ind]<< endl;

		}
	}