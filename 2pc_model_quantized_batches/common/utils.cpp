/*********************************************
Author: Mohamad Mansouri
Email: Mohamad.Mansouri@eurecom.fr
Date: 11/09/2018
**********************************************/

#include <iostream>
#include<cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include<dirent.h>
#include <string.h>
#include "utils.h"

using namespace std;

vector<vector<double>> extract_weight(string file_name)
{
	vector<double> tmp;
	vector<vector<double>> weight;
	ifstream infile(file_name);
	string line;
	double value;

	while (getline(infile, line))
	{
	    istringstream iss(line);
	    while (iss >> value)
				tmp.push_back(value);

		if (!(iss)) {weight.push_back(tmp) ; tmp.clear();} 
	}
	infile.close();
	return weight;
}


vector<double> extract_bias(string file_name)
{
	vector<double> bias;
	ifstream infile(file_name);
	string line;
	double value;

	while (getline(infile, line))
	{
	    istringstream iss(line);
	    while (iss >> value)
				bias.push_back(value);

		if (!(iss)) {break;} 
	}
	infile.close();
	return bias;

}


vector<vector<double>> extract_input(string path, vector<char>* truth)
{
	struct dirent *entry;
	vector<vector<double>> input;	
	DIR* pDIR = opendir(path.c_str());
	while((entry = readdir(pDIR)))	
		if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0  ){
			char t;
			if (entry->d_name[0] == '_')
				t = '/';
			else
				t = entry->d_name[0];
			truth->push_back(t);
			input.push_back(extract_bias(path + string(entry->d_name)));		
		}
	closedir(pDIR);
	return input;

}
