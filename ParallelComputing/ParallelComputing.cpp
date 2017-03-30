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
#include <chrono>
#include "Utils.h"

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

// Define the methods beforehand so that they can be used in any order.
void algorithmOne();
void readDataFromFile(std::string);
void writeProfiling(std::string);
float minimum(cl::Context, cl::CommandQueue, cl::Program);
float maximum(cl::Context, cl::CommandQueue, cl::Program);
float average(cl::Context, cl::CommandQueue, cl::Program, const std::vector<int>&);
float standard_deviation(cl::Context, cl::CommandQueue, cl::Program, float);

// Define the vectors to be used in the program to store the data.
std::vector<std::string> locations;
std::vector<int> years;
std::vector<int> months;
std::vector<int> days;
std::vector<std::string> times;
std::vector<int> temps;

// The total 
unsigned int total_time_kernel;
unsigned int total_time_memory;

using namespace std::chrono;

int main()
{
	high_resolution_clock::time_point exec_start_time, exec_end_time; //CPU clock times to measure the program execution time
	char userChoice;

	// Loop the Menu until '0' is entered by the user.
	//  At that point, the program will quit.
	while (true)
	{
		std::cout << std::string(50, '-') << std::endl;
		std::cout << "--> Sam D Kershaw - KER14468703" << std::endl;
		std::cout << "--> 1) Algorithm 1" << std::endl;
		std::cout << "--> 2) Algorithm 2" << std::endl;
		std::cout << "--> 0) Quit the Program" << std::endl;
		std::cin >> userChoice;
		std::cout << std::string(50, '-') << std::endl;
		switch (userChoice)
		{
		case '0':
			return 0;
		case '1':
			std::cout << "Running Algorithm One..." << std::endl;
			std::cout << std::string(50, '-') << std::endl;
			exec_start_time = high_resolution_clock::now(); // Get the initial CPU clock time
			algorithmOne();
			exec_end_time = high_resolution_clock::now(); // Get the finish CPU clock time
			break;
		default:
			std::cout << " ## Looks like you entered an illegal option... ## " << std::endl;
			continue;
			break;
		}
		// Measure the total program execution time in nanoseconds.
		auto duration = duration_cast<nanoseconds>(exec_end_time - exec_start_time).count();
		// Print out the total times to the console.
		std::cout << std::string(50, '-') << std::endl;
		std::cout << "--> Total program execution time: " << duration << " [ns]" << std::endl;
		std::cout << "--> Total kernel execution time: " << total_time_kernel << " [ns]" << std::endl;
		std::cout << "--> Total memory transfer time: " << total_time_memory << "[ns]" << std::endl;
		std::cout << std::string(50, '-') << std::endl;
		// Show 'Press any key to continue . . .' to the user.
		system("pause");
	}

	// End the program.
	return 0;
}

// Run the first algorithm.
void algorithmOne()
{
	// Set the device and platform IDs.
	int platform_id = 0;
	int device_id = 0;

	try {
		// Set up the OpenCL content and command queue.
		cl::Context context = GetContext(platform_id, device_id);
		std::cout << "--> Executing on " << GetPlatformName(platform_id) << ", " << GetDeviceName(platform_id, device_id) << std::endl;
		cl::CommandQueue queue(context, CL_QUEUE_PROFILING_ENABLE);

		// Load and build the device code.
		cl::Program::Sources sources;

		AddSources(sources, "kernel.cl");

		cl::Program program(context, sources);

		// Attempt to build the kernel code.
		try {
			program.build();
		}
		catch (const cl::Error& err) {
			std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << std::endl;
			std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << std::endl;
			std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << std::endl;
			throw err;
			return;
		}

		// Reserve space in the vectors for the data to be read from the text files.
		std::cout << "--> Reserving space in memory for vectors... ";
		locations.resize(DATASET_LENGTH);
		years.resize(DATASET_LENGTH);
		months.resize(DATASET_LENGTH);
		days.resize(DATASET_LENGTH);
		times.resize(DATASET_LENGTH);
		temps.resize(DATASET_LENGTH);
		std::cout << "Done." << std::endl;

		// Read in the data from the text file specified in the parameter.
		readDataFromFile("temp_lincolnshire.txt");
		
		// Calculate the values for each value.
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
		// Catch any errors that occur and display to the console.
		std::cerr << err.what() << std::endl;
	}
}

// Read in data from the text file specified in the filePath parameter.
void readDataFromFile(std::string filePath)
{
	std::ifstream txtFile(filePath); //Create an incoming file stream.

	// Create temporary variables to hold data as it is being read in.
	/*std::string location, time;
	int year, month, day;*/
	int temp;

	int lineCount = 0;

	std::cout << "--> Vector Size: " << temps.size() << std::endl;
	
	// Check if the text file can be opened.
	if (txtFile.is_open())
	{
		// Temporary variables to hold items from the text file.
		std::string item, line;

		std::cout << "--> Reading from '" << filePath << "'...";

		// Loop through each line in the text file.
		while (getline(txtFile, line))
		{
			std::string location;
			stringstream _line = stringstream(line);

			int count = 0;
			// Loop through each item in the current line, separated by a whitespace.
			while (getline(_line, item, ' '))
			{
				switch (count)
				{
				case 0:
					// Add item to the locations vector
					locations[lineCount] = item;
					count++;
					break;
				case 1:
					// Add item to the years vector
					years[lineCount] = stoi(item);
					count++;
					break;
				case 2:
					// Add item to the months vector
					months[lineCount] = stoi(item);
					count++;
					break;
				case 3:
					// Add item to the days vector
					days[lineCount] = stoi(item);
					count++;
					break;
				case 4:
					// Add item to the times vector
					times[lineCount] = item;
					count++;
					break;
				case 5:
					// Add item to the temps vector
					temps[lineCount] = stof(item);
					count = 0;
					lineCount++;
					break;
				default:
					count = 0;
					break;
				}
			}
		}
		std::cout << "\r--> Read " << lineCount << " lines from '" << filePath << "' successfully." << std::endl;
	}
	else {
		// If the file couldn't be opened, display an error to the user.
		std::cout << "Couldn't open the file!!";
	}

	// Close the text file stream.
	txtFile.close();
}

void writeProfiling(int kernel_exec_time, int mem_transfer_time)
{
	// Write the execution and transfer times to the console.
	std::cout << std::string(50, '-') << std::endl;
	std::cout << "-> Kernel Execution Time: " << kernel_exec_time << " [ns]" << std::endl;
	std::cout << "-> Memory Transfer Time: " << mem_transfer_time << " [ns]" << std::endl;
	// Add the times for this kernel to the total so far.
	total_time_kernel += kernel_exec_time; total_time_memory += mem_transfer_time;
}

// Function to get the smallest number in the vector.
float minimum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	std::vector<int> temp_temps = temps; //Create a temporary vector populated with data from the global temps vector.
	size_t local_size = 128; //Specify the size of the workgroups, as a power of 2.
	size_t padding_size = temp_temps.size() % local_size; //Check if the vector size is a multiple of the workgroup size.

	// Pad the vector if the size of the temp_temps vector isn't a multiple of 128.
	if (padding_size) {
		std::vector<int> temp(local_size - padding_size, INT_MAX);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end()); //Pad the vector with enough values to make it a multiple of 128.
	}

	size_t input = temp_temps.size(); //Get the capacity of the temp_temps vector.
	size_t input_size = temp_temps.size() * sizeof(int); //Get the actual size of the temp_temps vector.

	std::vector<int> min_val(1); //Create an output vector with size of 1.
	size_t output_size = min_val.size() * sizeof(int); //Get the output vector size.

	// Set up the input and output buffers.
	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	// Create events for the buffers, and kernel execution time.
	cl::Event write_event, fill_event, read_event, exec;

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0], NULL, &write_event);
	q.enqueueFillBuffer(output_buffer, INT_MAX, 0, output_size, NULL, &fill_event);

	// Define the kernel and add the arguments.
	cl::Kernel kernel = cl::Kernel(prg, "minimum");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(int)));

	// Run the kernel.
	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size), NULL, &exec);

	// Read in the data from the output buffer.
	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &min_val[0], NULL, &read_event);

	// Calculate execution times and print to console.
	int kernel_exec_time = exec.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		exec.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_write_time = write_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		write_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_fill_time = fill_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		fill_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_read_time = read_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		read_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_transfer_time = mem_write_time + mem_fill_time + mem_read_time;

	// Output the profiling info to the console.
	writeProfiling(kernel_exec_time, mem_transfer_time);

	// Return the minimum value.
	return (float)min_val[0];
}

// Returns the maximum value in the temps vector.
float maximum(cl::Context ctxt, cl::CommandQueue q, cl::Program prg)
{
	std::vector<int> temp_temps = temps; //Create a temporary vector populated with data from the global temps vector.
	size_t local_size = 128; //Specify the size of the workgroups, as a power of 2.
	size_t padding_size = temp_temps.size() % local_size; //Check if the vector size is a multiple of the workgroup size.

	// Pad the vector if the size of the temp_temps vector isn't a multiple of 128.
	if (padding_size) {
		std::vector<int> temp(local_size - padding_size, INT_MIN);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end()); //Pad the vector with enough values to make it a multiple of 128.
	}

	size_t input = temp_temps.size(); //Get the capacity of the temp_temps vector.
	size_t input_size = temp_temps.size() * sizeof(int); //Get the actual size of the temp_temps vector.

	std::vector<int> max_val(1); //Create an output vector with size of 1.
	size_t output_size = max_val.size() * sizeof(int); //Get the output vector size.

	// Set up the input and output buffers.
	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	// Create events for the buffers, and kernel execution time.
	cl::Event write_event, fill_event, read_event, exec;

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0], NULL, &write_event);
	q.enqueueFillBuffer(output_buffer, INT_MIN, 0, output_size, NULL, &fill_event);

	// Define the kernel and add the arguments.
	cl::Kernel kernel = cl::Kernel(prg, "maximum");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(int)));

	// Run the kernel.
	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size), NULL, &exec);

	// Read in the data from the output buffer.
	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &max_val[0], NULL, &read_event);

	// Calculate execution times and print to console.
	int kernel_exec_time = exec.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		exec.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_write_time = write_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		write_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_fill_time = fill_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		fill_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_read_time = read_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		read_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_transfer_time = mem_write_time + mem_fill_time + mem_read_time;

	// Output the profiling info to the console.
	writeProfiling(kernel_exec_time, mem_transfer_time);

	// Return the maximum value.
	return (float)max_val[0];
}

// Calculate the average number in the temp_temps vector.
float average(cl::Context ctxt, cl::CommandQueue q, cl::Program prg, const vector<int>& vec)
{
	// Work out if a valid vector has been passed as an argument.
	std::vector<int> temp_temps = (vec.size() == 0) ? temps : vec;

	size_t local_size = 128; //Create a temporary vector populated with data from the global temps vector.
	size_t padding_size = temp_temps.size() % local_size; //Specify the size of the workgroups, as a power of 2.

	//Check if the vector size is a multiple of the workgroup size.
	if (padding_size) {
		std::vector<int> temp(local_size - padding_size, 0);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end()); // Pad the vector if the size of the temp_temps vector isn't a multiple of 128.
	}

	size_t input = temp_temps.size(); //Get the capacity of the temp_temps vector.
	size_t input_size = temp_temps.size() * sizeof(int); //Get the actual size of the temp_temps vector.
	/*size_t nr_groups = input / local_size;*/

	std::vector<int> avg(1); //Create an output vector with size of 1.
	size_t output_size = avg.size()*sizeof(int); //Get the output vector size.


	// Set up the input and output buffers.
	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	// Create events for the buffers, and kernel execution time.
	cl::Event write_event, fill_event, read_event, exec;

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0], NULL, &write_event);
	q.enqueueFillBuffer(output_buffer, 0, 0, output_size, NULL, &fill_event);

	// Define the kernel and add the arguments.
	cl::Kernel kernel = cl::Kernel(prg, "sum");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(int)));

	// Run the kernel.
	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size), NULL, &exec);

	// Read in the data from the output buffer.
	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &avg[0], NULL, &read_event);

	// Calculate execution times and print to console.
	int kernel_exec_time = exec.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		exec.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_write_time = write_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		write_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_fill_time = fill_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		fill_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_read_time = read_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		read_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_transfer_time = mem_write_time + mem_fill_time + mem_read_time;

	// Output the profiling info to the console.
	writeProfiling(kernel_exec_time, mem_transfer_time);

	// Calculate and return the average value.
	return (float)avg[0] / (float)temp_temps.size();
}

// Function to calculate and return the standard deviation.
float standard_deviation(cl::Context ctxt, cl::CommandQueue q, cl::Program prg, float avg)
{
	std::vector<int> temp_temps = temps; //Create a temporary vector populated with data from the global temps vector.
	size_t local_size = 128; //Specify the size of the workgroups, as a power of 2.
	size_t padding_size = temp_temps.size() % local_size; //Check if the vector size is a multiple of the workgroup size.

	// Pad the vector if the size of the temp_temps vector isn't a multiple of 128.
	if (padding_size) {
		std::vector<int> temp(local_size - padding_size, FLT_MAX);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end()); //Pad the vector with enough values to make it a multiple of 128.
	}

	size_t input = temp_temps.size(); //Get the capacity of the temp_temps vector.
	size_t input_size = temp_temps.size() * sizeof(int); //Get the actual size of the temp_temps vector.
	/*size_t nr_groups = input / local_size;*/

	std::vector<int> sigma(DATASET_LENGTH); //Create an output vector with size of 1.
	size_t output_size = sigma.size() * sizeof(int); //Get the output vector size.

	// Set up the input and output buffers.
	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	// Create events for the buffers, and kernel execution time.
	cl::Event write_event, fill_event, read_event, exec;

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0], NULL, &write_event);
	q.enqueueFillBuffer(output_buffer, FLT_MAX, 0, output_size, NULL, &fill_event);

	// Define the kernel and add the arguments.
	cl::Kernel kernel = cl::Kernel(prg, "standard_dev");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, avg);
	kernel.setArg(3, cl::Local(local_size * sizeof(int)));

	// Run the kernel.
	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size), NULL, &exec);

	// Read in the data from the output buffer.
	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &sigma[0], NULL, &read_event);

	// Calculate execution times and print to console.
	int kernel_exec_time = exec.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		exec.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_write_time = write_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		write_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_fill_time = fill_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		fill_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_read_time = read_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
		read_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	int mem_transfer_time = mem_write_time + mem_fill_time + mem_read_time;

	// Output the profiling info to the console.
	writeProfiling(kernel_exec_time, mem_transfer_time);

	// Send the sigma vector to the average function, which will return the
	//  average value of that vector.
	float avg_sq_diffs = average(ctxt, q, prg, sigma);

	// Return the standard deviation.
	return sqrt(avg_sq_diffs);
}

// A function to calculate the median of the temps vector.
float median(cl::Context ctxt, cl::CommandQueue q, cl::Program prg) {
	std::vector<int> temp_temps = temps;
	size_t local_size = 128;
	size_t padding_size = temp_temps.size() % local_size;

	if (padding_size) {
		std::vector<int> temp(local_size - padding_size, INT_MAX);
		temp_temps.insert(temp_temps.end(), temp.begin(), temp.end());
	}

	size_t input = temp_temps.size();
	size_t input_size = temp_temps.size() * sizeof(int);

	std::vector<int> med(DATASET_LENGTH);
	size_t output_size = med.size() * sizeof(int);

	cl::Buffer input_buffer(ctxt, CL_MEM_READ_ONLY, input_size);
	cl::Buffer output_buffer(ctxt, CL_MEM_READ_WRITE, output_size);

	q.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, input_size, &temp_temps[0]);
	q.enqueueFillBuffer(output_buffer, INT_MAX, 0, output_size);

	cl::Kernel kernel = cl::Kernel(prg, "sort_asc");
	kernel.setArg(0, input_buffer);
	kernel.setArg(1, output_buffer);
	kernel.setArg(2, cl::Local(local_size * sizeof(int)));

	cl::Event profiler;
	q.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(input), cl::NDRange(local_size), NULL, &profiler);

	q.enqueueReadBuffer(output_buffer, CL_TRUE, 0, output_size, &med[0]);

	return 0.0;
}
