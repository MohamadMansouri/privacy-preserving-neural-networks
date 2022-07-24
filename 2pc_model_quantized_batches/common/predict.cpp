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
static BooleanCircuit* bc;
static BooleanCircuit* yc;

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
#else
const static uint32_t nvals_s = nvals_i;
#endif

#ifdef V1

// the precision of the inputs
#ifdef PCA
// const static uint32_t prec_i = 16;
const static uint32_t prec_pca = 24;
// const static uint32_t prec_wh = 16;
#else
const static uint32_t prec_pca = 0;
#endif

const static uint32_t prec_i = 24;
const static uint32_t prec_wh = 24;

const static uint32_t prec_h = prec_i * 2;
const static int shift = prec_h / 2;
const static uint32_t prec_wo = prec_h / 2;
const static uint32_t prec_o = prec_h;

#else

#ifdef PCA
const static uint32_t prec_i = 7;
const static uint32_t prec_pca = 6;
const static uint32_t prec_wh = 7;
#else
const static uint32_t prec_i = 10;
const static uint32_t prec_pca = 0;
const static uint32_t prec_wh = 10;
#endif

const static uint32_t prec_h = prec_i + prec_wh + prec_pca;
const static uint32_t prec_wo = 10;
const static uint32_t prec_o = prec_h * 2 + prec_wo;

#endif


// retrun an element of SIMD at certain index
share* extract_index(share *s_x , uint32_t i) 
{
	uint64_t zero = 0;
	share* s_dummy = ac->PutCONSGate(zero,bitlen);

	s_dummy->set_wire_id(0, s_x->get_wire_id(i));

	return s_dummy;
}


void convert_input_to_int(uint64_t arr[], vector<vector<double>> vec, uint32_t nvals, uint32_t n_sig, int prec)
{
	for (uint32_t k=0; k < n_sig; k++)
		for(uint32_t i=0; i < nvals; i++)
			arr[i + nvals*k] = (uint64_t)round(vec[k][i]*pow(2,prec));
}

void convert_vec_to_int(uint64_t arr[], vector<double> vec, uint32_t nvals, int prec)
{	
	for(uint32_t i=0;i<nvals;i++){
		arr[i] = (uint64_t) round(vec[i]*pow(2,prec));		
	}
}


void convert_mat_to_int(uint64_t arr[], vector<vector<double>> vec, uint32_t rows, uint32_t cols, int prec)
{
	for (uint32_t j=0;j<cols;j++)
		for(uint32_t i=0;i<rows;i++)
			arr[i+rows*j] = (uint64_t) round(vec[i][j]*pow(2,prec));
}


share* find_max(share* s_y, uint32_t nvals, uint32_t n_sig)
{
    uint64_t zero = 0x000000000000000;
    uint64_t ngtv_zero = 0x8000000000000000;

    share* s_zero = bc->PutSIMDCONSGate(n_sig, zero, bitlen);
    share* s_ngtv_zero = bc-> PutSIMDCONSGate(n_sig, ngtv_zero, bitlen);

    struct predicted s_result;

    uint32_t pos[n_sig];
    uint32_t k =0;
    for (uint32_t i=0 ; i < n_sig*nvals; i+=nvals){
        pos[k++] = i;
    }

    s_result.max = ac->PutSubsetGate(s_y, pos, n_sig);
    s_result.max = bc->PutA2BGate(s_result.max,yc);
    s_result.max = bc->PutMUXGate(s_zero,s_result.max,bc->PutGTGate(s_result.max, s_ngtv_zero));
    s_result.max_ind = bc->PutSIMDCONSGate(n_sig,zero,bitlen);

    share* tmp;
    for (uint32_t i = 1 ; i < nvals; i++) {
        k=0;

        for (uint32_t j = 0 ; j < n_sig * nvals; j += nvals)
            pos[k++] = j+i;


        uint64_t index = *(uint64_t*) &i;


        tmp = ac->PutSubsetGate(s_y, pos, n_sig);
        tmp = bc->PutA2BGate(tmp,yc);
        tmp = bc->PutMUXGate(s_zero,tmp,bc->PutGTGate(tmp, s_ngtv_zero));


        s_result.max = bc->PutMUXGate(s_result.max,tmp,bc->PutGTGate(s_result.max,tmp));
        s_result.max_ind = bc->PutMUXGate(s_result.max_ind,bc->PutSIMDCONSGate(n_sig,index,bitlen),bc->PutGTGate(s_result.max,tmp));

    }
    s_result.max_ind = ac->PutB2AGate(s_result.max_ind);

    return s_result.max_ind;

}


#ifdef V1
share* Shiffter(share* s_yh, int shift)
{
	
	s_yh = bc->PutA2BGate(s_yh,yc);
	
	vector<uint32_t> wires = s_yh->get_wires();

	for (uint32_t i=0 ; i< bitlen-shift-1 ; i++)
		wires[i] = wires[i+shift];

	for(uint32_t i=bitlen-2; i>bitlen-2-shift;i--)
		wires[i] = wires[bitlen-1];
	
	s_yh = new boolshare (wires,bc);

	return ac-> PutB2AGate(s_yh);
}
#endif

share* InnerProduct (share* s_input, share* s_weight, share* s_bias, uint32_t rows,uint32_t cols, uint32_t n_sig)
{

	// Multiplication gate
 	share* s_mult = ac->PutMULGate(s_input,s_weight);
	//sum
	uint32_t pos_id[cols*n_sig];
	int c = 0;
	
	for (uint32_t i = 0; i < rows*cols*n_sig; i+=rows)
		pos_id[c++] = i;

	share* s_sum = ac->PutSubsetGate(s_mult,pos_id,cols*n_sig);
	share* tmp;
	
	for (uint32_t i = 1; i < rows; i++) 
	{
		for (uint32_t j = 0; j < cols*n_sig; j++)
			pos_id[j]+=1; 

		tmp = ac->PutSubsetGate(s_mult,pos_id,cols*n_sig);
		s_sum = ac->PutADDGate(s_sum,tmp);

	}
	if (s_bias)
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

share* replicate(share* input, uint32_t nvals, uint32_t reps, uint32_t n_sig)
{
	if (n_sig == 1)
	{
		return replicate(input, reps);	
	}

	uint32_t pos_id [nvals];
	for (uint32_t i = 0; i < nvals; i++)
		pos_id[i] = i;

	share* s_reps = ac->PutSubsetGate(input, pos_id, nvals);

	s_reps = replicate(s_reps, reps);

	share* tmp;

	for (uint32_t i = 1; i < n_sig; i++)
	{
		for (uint32_t j = 0; j < nvals; j++)
			pos_id[j]+=nvals; 
		
		tmp = ac->PutSubsetGate(input, pos_id, nvals);
		tmp = replicate(tmp, reps);
		
		s_reps = ac->PutCombinerGate(s_reps,tmp);
	}

	return s_reps;
}


vector<char> predict(e_role role, const string& address, uint16_t port, seclvl seclvl, uint32_t nthreads,
	e_mt_gen_alg mt_alg, vector<vector<double>> input, vector<vector<double>> weight_all[2],
	 vector<double> bias_all[2], uint32_t n_sig
#ifdef PCA
	 ,vector<vector<double>> pca_all, vector<double> mean_all 
#endif
	 )
{

	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads, mt_alg, 12000000);

	vector<Sharing*>& sharings = party->GetSharings();

	ac = (ArithmeticCircuit*) sharings[S_ARITH]->GetCircuitBuildRoutine();
	bc = (BooleanCircuit*) sharings[S_BOOL]->GetCircuitBuildRoutine();
	yc = (BooleanCircuit*) sharings[S_YAO]->GetCircuitBuildRoutine();

	uint64_t input_arr[nvals_s*n_sig];
	uint64_t weight_arr_h[nvals_i*nvals_h];
	uint64_t bias_arr_h[nvals_h];

#ifdef PCA
	uint64_t pca_arr[nvals_s*nvals_i];
	uint64_t mean_arr[nvals_s];
#endif

	// Fill the inputs of each role to the arrays
	bool client = false;
	if (role == CLIENT){
		convert_input_to_int(input_arr, input, nvals_s, n_sig, prec_i);
		client = true;
	}

	if (role == SERVER)
		{
			convert_vec_to_int(bias_arr_h, bias_all[0], nvals_h, prec_h);
			convert_mat_to_int(weight_arr_h, weight_all[0],nvals_i,nvals_h,prec_wh);

#ifdef PCA			
			convert_vec_to_int(mean_arr,mean_all,nvals_s,prec_i);
			convert_mat_to_int(pca_arr, pca_all,nvals_s,nvals_i,prec_pca);
#endif
		}	

	share* s_input = ac->PutSIMDINGate(nvals_s*n_sig, input_arr, bitlen, CLIENT);


#ifdef PCA

	cout << "[-] Building PCA gates..." << endl;

	share* s_mean = ac->PutSIMDINGate(nvals_s, mean_arr, bitlen, SERVER);

	// Replicating mean
	s_mean = replicate(s_mean, n_sig);

	s_input = ac->PutADDGate(s_input,s_mean);


	share* s_pca = ac->PutSIMDINGate(nvals_s * nvals_i, pca_arr, bitlen, SERVER);


	// Replicating input
	s_input = replicate(s_input, nvals_s, nvals_i, n_sig);


	// Replicating pca
	s_pca = replicate(s_pca, n_sig);
	s_input = InnerProduct(s_input, s_pca, NULL, nvals_s, nvals_i, n_sig);
	
#ifdef V1
	s_input = Shiffter(s_input, shift);
#endif

#endif
	cout << "[-] Building Hidden Layer Circuit..." << endl;
	// Replicating input 
	s_input = replicate(s_input, nvals_i, nvals_h, n_sig);



	// Yh= I.Wh + Bh

	share* s_bias = ac->PutSIMDINGate(nvals_h,bias_arr_h,bitlen,SERVER);
	share* s_weight = ac->PutSIMDINGate(nvals_i*nvals_h, weight_arr_h, bitlen, SERVER);
	
	// Replicating bias and weights 
	s_bias = replicate(s_bias, n_sig);
	s_weight = replicate(s_weight, n_sig);


	
	share* s_yh= InnerProduct(s_input, s_weight, s_bias, nvals_i,nvals_h,n_sig);

#ifdef V1
	// truncate 
	s_yh = Shiffter(s_yh, shift);
#endif 

	// activation function (x2)
	s_yh = ac->PutMULGate(s_yh,s_yh);

#ifdef V1
	// truncate 
	s_yh = Shiffter(s_yh, shift);
#endif 
	
	
	cout << "[-] Building Output Layer..." << endl;

	// Y = Yh.Wo + Bo

	//Replicating yh
	s_yh = replicate(s_yh, nvals_h, nvals_o, n_sig);

	
	uint64_t bias_arr_o[nvals_o];
	uint64_t weight_arr_o[nvals_h*nvals_o];
	

	if (role == SERVER)
	{
		convert_vec_to_int(bias_arr_o,bias_all[1],nvals_o,prec_o);
		convert_mat_to_int(weight_arr_o, weight_all[1],nvals_h,nvals_o,prec_wo);
	}


	s_bias = ac->PutSIMDINGate(nvals_o,bias_arr_o,bitlen,SERVER);
	s_weight = ac->PutSIMDINGate(nvals_h*nvals_o, weight_arr_o, bitlen, SERVER);


	// Replicating bias and weights 
	s_bias = replicate(s_bias, n_sig);
	s_weight = replicate(s_weight, n_sig);

	share* s_y = InnerProduct(s_yh, s_weight,s_bias, nvals_h, nvals_o, n_sig);


	cout << "[-] Building SoftMax Layer......" << endl;
	// Max 
	// s_y = ac->PutSplitterGate(s_y);
	share* s_argmax = find_max(s_y,nvals_o,n_sig);
	s_argmax = ac->PutOUTGate(s_argmax ,CLIENT);



	cout << "[-] Start circuit execution..." << endl;

	party->ExecCircuit();

	// retrieve plain text output
	uint32_t out_bitlen, out_nvals;
	uint64_t *out_vals;

	vector<char> result;
	if (client)
	{		
		s_argmax->get_clear_value_vec(&out_vals, &out_bitlen, &out_nvals);
		for (uint32_t i = 0; i < out_nvals; i++) 
			// cout << classes[(int) out_vals[i]]<< endl;
			result.push_back(classes[(int) out_vals[i]]);
	}
	return result;
}
