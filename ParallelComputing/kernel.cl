__kernel void Min(__global const double* A, __global double* B, __local double* min)
{ 
	int id = get_global_id(0);
	int lid = get_local_id(0);
	int N = get_local_size(0);

	min[lid] = A[id];
	barrier(CLK_LOCAL_MEM_FENCE);

	for (int i = 1; i < N; i *= 2)
	{ 
		if (!(lid % (i * 2)) && ((lid + i) < N))
		{
			if (min[lid] > min[lid+i])
			{
				min[lid] = min[lid + i];
			}
			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}

	if (!lid)
	{
		atomic_min(&B[0], min[lid]);
	}
}