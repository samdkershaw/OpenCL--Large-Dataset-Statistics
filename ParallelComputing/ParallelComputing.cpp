// ParallelComputing.cpp : Defines the entry point for the console application.
//

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

//#define DATASET_LENGTH 18732
#define DATASET_LENGTH 1873106
//#define DATASET_LENGTH 20

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include "Utils.h"

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

void algorithmOne();
void readDataFromFile(std::string);
float minimum(cl::Context, cl::CommandQueue, cl::Program);
float maximum(cl::Context, cl::CommandQueue, cl::Program);
float average(cl::Context, cl::CommandQueue, cl::Program, const std::vector<int>&);
float standard_deviation(cl::Context, cl::CommandQueue, cl::Program, float);

typedef int mytype;
std::vector<std::string> locations;
std::vector<int> years;
std::vector<int> months;
std::vector<int> days;
std::vector<std::string> times;
std::vector<mytype> temps;

int main()
{
	char userChoice;
	do
	{
		std::cout << "--------------------------------------------------" << std::endl;
		std::cout << "--> Sam D Kershaw - KER14468703" << std::endl;
		std::cout << "--> 1) Algorithm 1" << std::endl;
		std::cout << "--> 2) Algorithm 2" << std::endl;
		std::cout << "--> 0) Quit the Program" << std::endl;
		std::cin >> userChoice;
		std::cout << "--------------------------------------------------" << std::endl;
		switch (userChoice)
		{
		case '1':
			std::cout << "Running Algorithm One..." << std::endl;
			algorithmOne();
			break;
		default:
			break;
		}
	} while (userChoice != '0');

	return 0;
}

void algorithmOne()
{
	int platform_id = 0;
	int device_id = 0;

	try {
		cl::Context context = GetContext(platform_id, device_id);
		std::cout << "--> Executing on " << GetPlatformName(platform_id) << ", " << GetDeviceName(platform_id, device_id) << std::endl;
		cl::CommandQueue queue(context, CL_QUEUE_PROFILING_ENABLE);

		//2.2 Load & build the device code
		cl::Program::Sources sources;

		AddSources(sources, "kernel.cl");

		cl::Program program(context, sources);

		//build and debug the kernel code
		try {
			program.build();
		}
		catch (const cl::Error& err) {
			std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << std::endl;
			std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << std::endl;
			std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << std::endl;
			throw err;
		}

		std::cout << "--> Reserving space in memory for vectors... ";
		locations.resize(DATASET_LENGTH);
		years.resize(DATASET_LENGTH);
		months.resize(DATASET_LENGTH);
		days.resize(DATASET_LENGTH);
		times.resize(DATASET_LENGTH);
		temps.resize(DATASET_LENGTH);
		std::cout << "Done." << std::endl;

		readDataFromFile("temp_lincolnshire.txt");
		
		float min_val = minimum(context, queue, program);
		std::cout << "--> Minimum Value: " << min_val << std::endl;
		float max = maximum(context, queue, program);
		std::cout << "--> Maximum Value: " << max << std::endl;
		float avg = average(context, queue, program, std::vector<int>());
		std::cout << "--> Average Value: " << avg << std::endl;
		float standard_dev = standard_deviation(context, queue, program, avg);
		std::cout << "--> Standard Deviation: " << standard_dev << std::endl;
		
		//std::cout << "Median Value: " << median << std::endl;
	}
	catch (cl::Error err) {
		std::cerr << err.what() << std::endl;
	}
}

void readDataFromFile(std::string filePath)
{
	std::ifstream txtFile(filePath);

	std::string location, time;
	int year, month, day;
	mytype temp;

	int lineCount = 0;

	std::cout << "--> Vector Size: " << temps.size() << std::endl;
	
	if (txtFile.is_open())
	{
		std::string item, line;

		std::cout << "--> Reading from '" << filePath << "'...";

		while (getline(txtFile, line))
		{
			std::string location;
			stringstream _line = stringstream(line);

			int count = 0;
			while (getline(_line, item, ' '))
			{
				switch (count)
				{
				case 0:
					//location
					locations[lineCount] = item;
					count++;
					break;
				case 1:
					//year
					years[lineCount] = stoi(item);
					count++;
					break;
				case 2:
					//month
					months[lineCount] = stoi(item);
					count++;
					break;
				case 3:
					days[lineCount] = stoi(item);
					count++;
					break;
				case 4:
					times[lineCount] = item;
					count++;
					break;
				case 5:
					temps[lineCount] = stof(item);
					count = 0;
					break;
				default:
					count = 0;
					break;
				}
			}

			lineCount++;
		}
	}
	else {
		std::cout << "Couldn't open the file!!";
	}

	txtFile.close();

	/*while (txtFile >> location >> year >> month >> day >> time >> temp) {
		locations.push_back(location);
		years.push_back(year);
		months.push_back(month);
		days.push_back(day);
		times.push_back(time);
		temps.push_back(temp);
	}*/

	std::cout << "\r--> Read " << DATASET_LENGTH << " lines from '" << filePath << "' successfully." << std::endl;
}

float minimum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	std::vector<mytype> temp_temps = temps;
	size_t local_size = 128;
	size_t padding_size = temp_temps.size() % local_size;

	if (padding_size) {
		std::vector<mytype> temp(local_size - padding_size, INT_MAX);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end());
	}

	size_t input = temp_temps.size();
	size_t input_size = temp_temps.size() * sizeof(mytype);

	std::vector<mytype> min_val(1);
	size_t output_size = min_val.size() * sizeof(mytype);

	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0]);
	q.enqueueFillBuffer(output_buffer, INT_MAX, 0, output_size);

	cl::Kernel kernel = cl::Kernel(prg, "minimum");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(mytype)));

	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size));

	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &min_val[0]);

	return min_val[0];
}

float maximum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	std::vector<mytype> temp_temps = temps;
	size_t local_size = 128;
	size_t padding_size = temp_temps.size() % local_size;

	if (padding_size) {
		std::vector<mytype> temp(local_size - padding_size, INT_MIN);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end());
	}

	size_t input = temp_temps.size();
	size_t input_size = temp_temps.size() * sizeof(mytype);

	std::vector<mytype> max_val(1);
	size_t output_size = max_val.size() * sizeof(mytype);

	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0]);
	q.enqueueFillBuffer(output_buffer, INT_MIN, 0, output_size);

	cl::Kernel kernel = cl::Kernel(prg, "maximum");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(mytype)));

	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size));

	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &max_val[0]);
	return (float)max_val[0];
}

float average(cl::Context ctxt, cl::CommandQueue q, cl::Program prg, const vector<int>& vec=vector<int>())
{
	std::vector<mytype> temp_temps = (vec.size() == 0) ? temps : vec;

	size_t local_size = 128;
	size_t padding_size = temp_temps.size() % local_size;

	if (padding_size) {
		std::vector<mytype> temp(local_size - padding_size, 0);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end());
	}

	size_t input = temp_temps.size();
	size_t input_size = temp_temps.size() * sizeof(mytype);
	size_t nr_groups = input / local_size;

	std::vector<mytype> avg(1);
	size_t output_size = avg.size()*sizeof(mytype);

	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0]);
	q.enqueueFillBuffer(output_buffer, 0, 0, output_size);

	cl::Kernel kernel = cl::Kernel(prg, "sum");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(mytype)));

	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size));

	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &avg[0]);

	return (float)avg[0] / (float)temp_temps.size();
}

float standard_deviation(cl::Context ctxt, cl::CommandQueue q, cl::Program prg, float avg)
{
	std::vector<mytype> temp_temps = temps;
	size_t local_size = 128;
	size_t padding_size = temp_temps.size() % local_size;

	if (padding_size) {
		std::vector<mytype> temp(local_size - padding_size, FLT_MAX);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end());
	}

	size_t input = temp_temps.size();
	size_t input_size = temp_temps.size() * sizeof(mytype);
	size_t nr_groups = input / local_size;

	std::vector<mytype> sigma(DATASET_LENGTH);
	size_t output_size = sigma.size() * sizeof(mytype);

	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0]);
	q.enqueueFillBuffer(output_buffer, FLT_MAX, 0, output_size);

	cl::Kernel kernel = cl::Kernel(prg, "standard_dev");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, avg);
	kernel.setArg(3, cl::Local(local_size * sizeof(mytype)));

	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size));

	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &sigma[0]);

	std::cout << "Average Given: " << avg << std::endl;
	std::cout << "Squared Difference: " << sigma[5173] << std::endl;

	float stage_three_avg = average(ctxt, q, prg, sigma);

	std::cout << "Stage 3 avg: " << stage_three_avg << std::endl;

	return sqrt(stage_three_avg);
}
