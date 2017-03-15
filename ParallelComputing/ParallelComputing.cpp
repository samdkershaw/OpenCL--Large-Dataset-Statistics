// ParallelComputing.cpp : Defines the entry point for the console application.
//

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <vector>
#include <string>
#include "Utils.h"

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

void readInData();
void printMenu();
int getMenuChoice();

int main()
{
	int platform_id = 0;
	int device_id = 0;

	int userChoice = getMenuChoice();

	cl::Context context = GetContext(platform_id, device_id);

	cl::CommandQueue queue(context);

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


    return 0;
}

void readInData()
{
}

int getMenuChoice(bool clear)
{
	int userChoice = -1;
	while (userChoice < 1 || userChoice > 3)
	{
		printMenu(false);
		if (!(cin >> userChoice))
		{
			cin.clear(); //clear the failure flag if there is an error when reading!
			string garbage;
			std::getline(cin, garbage); //read the garbage from the stream and throw it away
		}
	}
		
}

void printMenu(bool clear)
{
	if (clear) {
		system("cls");
	}
	std::cout << "Parallel Computing ~ Assignment/r/r" << std::endl;
	std::cout << "1 - Menu Item 1" << std::endl;
	std::cout << std::endl;
	std::cout << "> ";
}