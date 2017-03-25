// ParallelComputing.cpp : Defines the entry point for the console application.
//

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

#define DATASET_LENGTH 18732 

#include <iostream>
#include <vector>
#include <string>
#include "Utils.h"

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

void runOperations(int);
void algorithmOne();
void readDataFromFile(std::string);
double minimum(cl::Context, cl::CommandQueue, cl::Program);
double maximum(cl::Context, cl::CommandQueue, cl::Program);
double average(cl::Context, cl::CommandQueue, cl::Program);
double standard_deviation(cl::Context, cl::CommandQueue, cl::Program, double);

std::vector<std::string> locations;
std::vector<int> years;
std::vector<int> months;
std::vector<int> days;
std::vector<double> temps;

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

void runOperations(int algorithm=1) {
	
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

		readDataFromFile("temp_lincolnshire_short.txt");
		
		double min = minimum(context, queue, program);
		std::cout << "--> Minimum Value: " << min << std::endl;
		double max = maximum(context, queue, program);
		std::cout << "--> Maximum Value: " << max << std::endl;
		double avg = average(context, queue, program);
		std::cout << "--> Average Value: " << avg << std::endl;
		double standard_dev = standard_deviation(context, queue, program, avg);
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
	
	int lineCount = 0;

	if (txtFile.is_open()) 
	{
		std::string item, line;

		std::cout << "Lines Read: " << lineCount << "...";

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
					locations.push_back(item);
					count++;
					break;
				case 1:
					//year
					years.push_back(stoi(item));
					count++;
					break;
				case 2:
					//month
					months.push_back(stoi(item));
					count++;
					break;
				case 3:
					days.push_back(stoi(item));
					count++;
				case 4:
					temps.push_back(stod(item));
					count = 0;
					break;
				default:
					count = 0;
					break;
				}
			}

			lineCount++;

			if (lineCount % 100000 == 0) {
				std::cout << "\rLines Read: " << lineCount << "....";
			}
		}
	}
	else {
		std::cout << "Couldn't open the file!!";
	}

	txtFile.close();

	std::cout << "\rRead " << lineCount << " lines in successfully." << std::endl;
}

double minimum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	// Sets vector, local size and padding size
	vector<double> tempTempTemp = temps;
	size_t localSize = 256;
	size_t paddingSize = tempTempTemp.size() % localSize;

	// Adds padding to the vector
	if (paddingSize)
	{
		std::vector<double> temp(localSize - paddingSize, INT_MAX);
		tempTempTemp.insert(tempTempTemp.end(), temp.begin(), temp.end());
	}

	// Sets the input elemtents and size of input
	size_t inputElements = tempTempTemp.size();
	size_t inputSize = tempTempTemp.size() * sizeof(double);

	// Sets sizes of output
	std::vector<double> min(1);
	size_t outputSize = sizeof(double);

	// Sets the input and outout buffers
	cl::Buffer inputBuffer(ctxt, CL_MEM_READ_ONLY, inputSize);
	cl::Buffer output(ctxt, CL_MEM_READ_WRITE, outputSize);

	// Moves buffer to the GPU
	q.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, inputSize, &tempTempTemp[0]);
	q.enqueueFillBuffer(output, INT_MAX, 0, outputSize);

	// Sets the kernel and the arguments for the kernel
	cl::Kernel kernel = cl::Kernel(prg, "Min");
	kernel.setArg(0, inputBuffer);
	kernel.setArg(1, output);
	kernel.setArg(2, cl::Local(localSize * sizeof(double)));

	// Runs the kernel with a thread for each element of the input with workgroups sized the same as local size
	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(inputElements), cl::NDRange(localSize));

	// Reads the output from GPU to CPU
	q.enqueueReadBuffer(output, CL_TRUE, 0, outputSize, &min[0]);

	// Prints and retunes Minimum value
	std::cout << "Minimum: " << (double)min.at(0) / (double)10 << endl;
	return min.at(0);
}

double maximum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	return 0.0;
}

double average(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	return 0.0;
}

double standard_deviation(cl::Context ctxt, cl::CommandQueue q, cl::Program prg, double avg)
{
	return 0.0;
}
