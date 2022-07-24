/*********************************************
Author: Mohamad Mansouri
Email: Mohamad.Mansouri@eurecom.fr
Date: 11/09/2018
**********************************************/

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "utils.h"

std::vector<std::vector<double>> extract_weight(std::string file_name)
{
	std::vector<double> tmp;
	std::vector<std::vector<double>> weight;
	std::ifstream infile(file_name);
	std::string line;
	double value;

	while (std::getline(infile, line))
	{
	    std::istringstream iss(line);
	    while (iss >> value)
				tmp.push_back(value);

		if (!(iss)) {weight.push_back(tmp) ; tmp.clear();} 
	}
	infile.close();
	return weight;
}


std::vector<double> extract_bias(std::string file_name)
{
	std::vector<double> bias;
	std::ifstream infile(file_name);
	std::string line;
	double value;

	while (std::getline(infile, line))
	{
	    std::istringstream iss(line);
	    while (iss >> value)
				bias.push_back(value);

		if (!(iss)) {break;} 
	}
	infile.close();
	return bias;

}
