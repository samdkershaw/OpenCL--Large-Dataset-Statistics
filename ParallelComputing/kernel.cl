// Kernel to find the minimum number in a vector.
__kernel void minimum(__global const int* A, __global int* B, __local int* min)
{ 
	int gid = get_global_id(0); // Get the ID of the item in global memory
	int lid = get_local_id(0); // Get the ID of the item in local memory
	int lsize = get_local_size(0); // Get the size of the workgroups

	min[lid] = A[gid]; // Write from the input vector to a local array
	barrier(CLK_LOCAL_MEM_FENCE); // Wait for all operations to reach this point

	// 
	for (int i = 1; i < lsize; i *= 2)
	{ 
		if (!(lid % (i * 2)) && ((lid + i) < lsize))
		{
			// Compare the current value to the other values.
			min[lid] = (min[lid]>min[lid+i]) ? min[lid+i] : min[lid];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// The minimum value in each workgroup is compared with others
	if (lid == 0)
	{
		atomic_min(&B[0], min[lid]);
	}
}

// Kernel to find the largest number in a vector.
__kernel void maximum(__global const int* A, __global int* B, __local int* max) {
	int gid = get_global_id(0); // Get the ID of the item in global memory
	int lid = get_local_id(0); // Get the ID of the item in local memory
	int lsize = get_local_size(0); // Get the size of the workgroups

	max[lid] = A[gid]; //Read into local memory from 
	barrier(CLK_LOCAL_MEM_FENCE); //Wait for all local operations to reach this point


	for (int i = 1; i < lsize; i *= 2)
	{ 
		if (!(lid % (i * 2)) && ((lid + i) < lsize))
		{
			// Compare the current value to the other values
			max[lid] = (max[lid]<max[lid+i]) ? max[lid+i] : max[lid];
		}
		barrier(CLK_LOCAL_MEM_FENCE); //Wait for all local operations to reach this point
	}

	// The maximum value in each workgroup is compared with others
	if (lid == 0)
	{
		atomic_max(&B[0], max[lid]);
	}
}

// Kernel to add all the items in the vector together
__kernel void sum(__global const int* A, __global int* B, __local int* scratch) {
	int gid = get_global_id(0); // Get the ID of the item in global memory
	int lid = get_local_id(0); // Get the ID of the item in local memory
	int lsize = get_local_size(0); // Get the size of the workgroups

	scratch[lid] = A[gid]; //Write from the global array to the local one

	barrier(CLK_LOCAL_MEM_FENCE); //Wait for all local operations to reach this point

	/* * 2)) && ((lid + i) < lsize)) {*/

	for (int i = 1; i < lsize; i *= 2) {
		if (!(lid % (i*2)) && ((lid + i) < lsize)) {
			// Add the current value to the current total
			scratch[lid] += scratch[lid+i];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// The first element in each workgroup is combined.
	if (lid == 0) {
		atom_add(&B[0], scratch[lid]);
	}
}

// Kernel to calculate the average taken from the current value, and then squared,
//  the first few steps to obtain the standard deviation.
__kernel void standard_dev(__global const int* A, __global int* B, float avgVal, __local float* std_dev) {
	int gid = get_global_id(0); // Get the ID of the item in global memory
	int lid = get_local_id(0); // Get the ID of the item in local memory

	// Calculate the average taken from the current value, and then squared and assigned to the output vector
	B[gid] = pow((A[gid] - avgVal), 2);
}

//Kernel to 
__kernel void bitonic_sort(__global const float* A, __global float* B, __local float* aux) {
	int gid = get_global_id(0);
	int wg_bits = 2*get_local_size(0) - 1;

	/*for (int inc=*/
}
