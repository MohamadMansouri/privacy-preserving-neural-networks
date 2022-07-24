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

#include <abycore/sharing/sharing.h>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common/predict.h"
#include "common/utils.h"

using namespace std;

void read_test_options(int32_t* argcp, char*** argvp,
	uint32_t* secparam, string* address, uint16_t* port ,
	 string* circuit, uint32_t* n_signal) {

	uint32_t int_port = 0;

	parsing_ctx options[] =
	{
	{(void*) n_signal, T_NUM, "i", "Input Size", true, false },
	{(void*) secparam, T_NUM, "s", "Symmetric Security Bits, default: 128", false, false },
	{(void*) address, T_STR, "a", "IP-address, default: localhost", false, false },
	{(void*) circuit, T_STR, "c", "circuit file name", false, false },
	{(void*) &int_port, T_NUM, "p", "Port, default: 7766", false, false }

	};

	if (!parse_options(argcp, argvp, options,
		sizeof(options) / sizeof(parsing_ctx))) {
		print_usage(*argvp[0], options, sizeof(options) / sizeof(parsing_ctx));
	cout << "Exiting" << endl;	
	exit(0);

	}

	

	if (int_port != 0) {
		assert(int_port < 1 << (sizeof(uint16_t) * 8));
		*port = (uint16_t) int_port;
	}

}




int main(int argc, char** argv) {

	uint32_t  secparam = 128, nthreads = 1;

	uint16_t port = 7766;
	uint32_t n_sig = 1;

	string address = "127.0.0.1";
	string circuit = "none.aby";
	e_mt_gen_alg mt_alg = MT_OT;
	

	vector<vector<double>> dummy_weight[2];
	vector<double> dummy_bias[2];	
	vector<vector<double>> input;
	vector<char> truth;

#ifdef PCA
	vector<vector<double>> dummy_pca;
	vector<double> dummy_mean;	
	string path = "./input/signals/";
#else
	string path = "./input/signal_pca/";
#endif

	read_test_options(&argc, &argv, &secparam, &address,
		&port, &circuit, &n_sig);

	input = extract_input( path, &truth );

	seclvl seclvl = get_sec_lvl(secparam);

	vector<char> predicted;
	for (size_t i=0; i< /*input.size() - */n_sig; i+=n_sig)
	{
		vector<vector<double>>::const_iterator first = input.begin() + i ;
		vector<vector<double>>::const_iterator last = input.begin() + i + n_sig ;
		vector<vector<double>> sub_input(first, last);
		
		vector<char> r;
		
		r = predict(CLIENT, address, port, seclvl,  nthreads, mt_alg, sub_input, dummy_weight,dummy_bias,n_sig
#ifdef PCA
		, dummy_pca, dummy_mean
#endif
		);
		predicted.insert(predicted.end(), r.begin(), r.end()); 
	}

	streambuf* buf;

#ifdef SAVE
	const string model_path =  "../results/2pc";

		
	mkdir(model_path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

#ifdef PCA
	const string pca_path =  model_path + "/pca_in_2pc";
#else
	const string pca_path =  model_path + "/pca_before_2pc";
#endif

#ifdef V1
	const string file =  pca_path + "/truth_predict_v1";
#else
	const string file =  pca_path + "/truth_predict_v2";
#endif


	if (mkdir(pca_path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1 && errno != EEXIST)
		buf = cout.rdbuf();
	else
	{
		ofstream of;
		of = ofstream(file);
		buf = of.rdbuf();
	}

#else
	buf = cout.rdbuf();
#endif	 

	ostream  os(buf);
	for (size_t i =0; i < predicted.size(); i++)
	{
		os << truth[i] << " " << predicted[i] << endl;
	}
	
	return 0;

}
