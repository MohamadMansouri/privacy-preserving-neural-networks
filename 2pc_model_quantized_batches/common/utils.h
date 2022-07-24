/*********************************************
Author: Mohamad Mansouri
Email: Mohamad.Mansouri@eurecom.fr
Date: 11/09/2018
**********************************************/

#ifndef _UTILS_
#define _UTILS_

#include<cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include<dirent.h>

using namespace std;
// Method to extract weights from files to vectors.
vector<vector<double>> extract_weight(string file_name);

// Method to extract bias and input from files to vectors.
vector<double> extract_bias(string file_name);

vector<vector<double>> extract_input(string path, vector<char>* truth);

#endif