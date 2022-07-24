/*********************************************
Author: Mohamad Mansouri
Email: Mohamad.Mansouri@eurecom.fr
Date: 11/09/2018
**********************************************/

#ifndef _UTILS_
#define _UTILS_
#include <vector>
#include <string>
// Method to extract weights from files to vectors.
std::vector<std::vector<double>> extract_weight(std::string file_name);

// Method to extract bias and input from files to vectors.
std::vector<double> extract_bias(std::string file_name);


#endif