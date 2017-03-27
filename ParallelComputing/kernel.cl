//__kernel void Min(__global const double* A, __global double* B, __local double* min)
//{ 
//	int id = get_global_id(0);
//	int lid = get_local_id(0);
//	int N = get_local_size(0);
//
//	min[lid] = A[id];
//	barrier(CLK_LOCAL_MEM_FENCE);
//
//	for (int i = 1; i < N; i *= 2)
//	{ 
//		if (!(lid % (i * 2)) && ((lid + i) < N))
//		{
//			if (min[lid] > min[lid+i])
//			{
//				min[lid] = min[lid + i];
//			}
//			barrier(CLK_LOCAL_MEM_FENCE);
//		}
//	}
//
//	if (!lid)
//	{
//		atomic_min(&B[0], min[lid]);
//	}
//}

__kernel void average(__global const int* A, __global int* B, __local int* scratch) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	int lsize = get_local_size(0);

	scratch[lid] = A[gid];

	barrier(CLK_LOCAL_MEM_FENCE);

	/* * 2)) && ((lid + i) < lsize)) {*/

	for (int i = 1; i < lsize; i *= 2) {
		if (!(lid % (i*2)) && ((lid + i) < lsize)) {
			scratch[lid] += scratch[lid+i];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (!lid) {
		atomic_add(&B[0],scratch[lid]);
	}
}

//__kernel void average(__global const float* A, __global float* B, __local float* local_holder) {
//	int gid = get_global_id(0);
//	int lid = get_local_id(0);
//	int gsize = get_global_size(0);
//	int lsize = get_local_size(0);
//
//	local_holder[lid] = A[gid];
//
//	barrier(CLK_LOCAL_MEM_FENCE);
//	for (int i = 0; i < lsize; i*=2) {
//		if (!(lid % (i*2)) && ((lid + i) < lsize)) {
//			local_holder[lid] += local_holder[lid + i];
//		}
//		barrier(CLK_LOCAL_MEM_FENCE);
//	}
//
//	if (!lid) {
//		local_holder[lid] /= gsize;
//	}
//}