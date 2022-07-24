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

#include "common/predict.h"
#include "common/utils.h"

using namespace std;

void read_test_options(int32_t* argcp, char*** argvp,
	uint32_t* secparam, string* address,
	uint16_t* port , string* circuit, string* input) {

	uint32_t int_port = 0;

	parsing_ctx options[] =
	{
	{(void*) input, T_STR, "s", "signal file", true, false },
	{(void*) secparam, T_NUM, "s", "Symmetric Security Bits, default: 128", false, false },
	{(void*) address, T_STR, "a", "IP-address, default: localhost", false, false },
	{(void*) circuit, T_STR, "c", "circuit file name", false, false },
	{(void*) &int_port, T_NUM, "p", "Port, default: 7766", false, false }

	};

	if (!parse_options(argcp, argvp, options,
		sizeof(options) / sizeof(parsing_ctx))) {
		print_usage(*argvp[0], options, sizeof(options) / sizeof(parsing_ctx));
	cout << "Exiting" << endl;	
	cout <<  *input << endl;
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
	string address = "127.0.0.1";
	string circuit = "none.aby";
	e_mt_gen_alg mt_alg = MT_OT;
	string signal_file;
	vector<double> input;
	vector<vector<double>> dummy_weight[2];
	vector<double> dummy_bias[2];	
	vector<vector<double>> dummy_pca;
	vector<double> dummy_mean;	
	
	read_test_options(&argc, &argv, &secparam, &address,
		&port, &circuit, &signal_file);

	input = extract_bias( signal_file );

	seclvl seclvl = get_sec_lvl(secparam);
	cout.precision(10);


	predict(CLIENT, address, port, seclvl,  nthreads, mt_alg, input, dummy_weight,dummy_bias
#ifdef PCA
		, dummy_pca, dummy_mean
#endif
		);

	return 0;
}
