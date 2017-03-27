// ParallelComputing.cpp : Defines the entry point for the console application.
//

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

#define DATASET_LENGTH 18732

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
float average(cl::Context, cl::CommandQueue, cl::Program);
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
	int platform_id = 1;
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

		readDataFromFile("temp_lincolnshire_short.txt");
		
		float min = minimum(context, queue, program);
		std::cout << "--> Minimum Value: " << min << std::endl;
		float max = maximum(context, queue, program);
		std::cout << "--> Maximum Value: " << max << std::endl;
		float avg = average(context, queue, program);
		std::cout << "--> Average Value: " << avg << std::endl;
		float standard_dev = standard_deviation(context, queue, program, avg);
		std::cout << "--> Standard Deviation: " << standard_dev << std::endl;
		
		//std::cout << "Median Value: " << median << std::endl;
	}
	catch (cl::Error err) {
		std::cerr << err.what() << std::endl;
	}
	system("pause");
}

void readDataFromFile(std::string filePath)
{
	std::ifstream txtFile(filePath);

	std::string location, time;
	int year, month, day;
	mytype temp;

	int lineCount = 0;

	std::cout << "--> Vector Size: " << temps.size() << std::endl;

	for (int i = 0; i < DATASET_LENGTH; i++) {
		txtFile >> locations[i] >> years[i] >> months[i] >> days[i] >> times[i] >> temps[i];
	}

	/*while (txtFile >> location >> year >> month >> day >> time >> temp) {
		locations.push_back(location);
		years.push_back(year);
		months.push_back(month);
		days.push_back(day);
		times.push_back(time);
		temps.push_back(temp);
	}*/

	txtFile.close();

	std::cout << "\rRead " << DATASET_LENGTH << " lines in successfully." << std::endl;
	std::cout << "--> Test: " << temps[DATASET_LENGTH-1] << std::endl;
	system("pause");
}

float minimum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	return 0.0;
}

float maximum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	return 0.0;
}

float average(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	std::vector<mytype> temp_temps = temps;
	std::cout << "temp_temps.size() = " << temp_temps.size() << std::endl;
	size_t local_size = 256;
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

	cl::Kernel kernel = cl::Kernel(prg, "average");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(mytype)));

	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size));

	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &avg[0]);

	return ((float)avg[0]/(float)10) / (float)temp_temps.size();
}

float standard_deviation(cl::Context ctxt, cl::CommandQueue q, cl::Program prg, float avg)
{
	return 0.0;
}
