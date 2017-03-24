#include "stdafx.h"
#include "DataSet.h"


DataSet::DataSet()
{
}

DataSet::DataSet(std::string* locations, short* years, short* months, short* days, double* temps)
{
	_locations = locations;
	_years = years;
	_months = months;
	_days = days;
	_temps = temps;
}

DataSet::DataSet(std::vector<std::string> locations, std::vector<short> years, std::vector<short> months,
	std::vector<short> days, std::vector<double> temps)
{
	
}

DataSet::~DataSet()
{
}

void DataSet::printLn(int lnNum)
{
	std::cout << "Location: " + _locations[lnNum] << std::endl;
	std::cout << "Year: " + _years[lnNum] << std::endl;
	std::cout << "Month: " + _months[lnNum] << std::endl;
	std::cout << "Day: " + _days[lnNum] << std::endl;
	std::cout << "Temp: " + (int)_temps[lnNum] << std::endl;
}
