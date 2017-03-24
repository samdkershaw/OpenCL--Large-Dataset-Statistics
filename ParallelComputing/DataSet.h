#pragma once
#include <string>
#include <iostream>
#include <vector>
class DataSet
{
public:
	DataSet();
	DataSet(std::string*, short*, short*, short*, double*);
	DataSet(std::vector<std::string>, std::vector<short>, std::vector<short>, std::vector<short>, std::vector<double>);
	~DataSet();
	void printLn(int);
protected:
	std::string *_locations;
	short *_years, *_months, *_days;
	double *_temps;
};

