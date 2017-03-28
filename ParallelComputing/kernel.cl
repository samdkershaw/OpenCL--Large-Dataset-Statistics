__kernel void minimum(__global const int* A, __global int* B, __local int* min)
{ 
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	int lsize = get_local_size(0);

	min[lid] = A[gid];
	barrier(CLK_LOCAL_MEM_FENCE);

	for (int i = 1; i < lsize; i *= 2)
	{ 
		if (!(lid % (i * 2)) && ((lid + i) < lsize))
		{
			min[lid] = (min[lid]>min[lid+i]) ? min[lid+i] : min[lid];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (lid == 0)
	{
		atomic_min(&B[0], min[lid]);
	}
}

__kernel void maximum(__global const int* A, __global int* B, __local int* max) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	int lsize = get_local_size(0);

	max[lid] = A[gid];
	barrier(CLK_LOCAL_MEM_FENCE);

	for (int i = 1; i < lsize; i *= 2)
	{ 
		if (!(lid % (i * 2)) && ((lid + i) < lsize))
		{
			max[lid] = (max[lid]<max[lid+i]) ? max[lid+i] : max[lid];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (lid == 0)
	{
		atomic_max(&B[0], max[lid]);
	}
}

__kernel void sum(__global const int* A, __global int* B, __local int* scratch) {
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

	if (lid == 0) {
		atom_add(&B[0], scratch[lid]);
	}
}

void cmpxchg(__global int* A, __global int* B, bool desc) {
	if ((!desc && *A > *B) || (desc && *A < *B)) {
		int t = *A; *A = *B; *B = t;
	}
}

void bitonic_merge(int id, __global int* A, int gsize, bool desc) {
	for (int i = gsize/2; i > 0; i/=2) {
		if ((id % (i*2)) < i)
			cmpxchg(&A[id],&A[id+i],desc);

		barrier(CLK_GLOBAL_MEM_FENCE);
	}
}

__kernel void sort_bitonic_asc(__global int* A) {
	int gid = get_global_id(0);
	int gsize = get_global_size(0);


	for (int i = 1; i < gsize/2; i*=2) {
		if (gid % (i*4) < i*2)
			bitonic_merge(gid, A, i*2, false);
		else if ((gid + i*2) % (i*4) < i*2)
			bitonic_merge(gid, A, i*2, true);
		barrier(CLK_GLOBAL_MEM_FENCE);
	}

	bitonic_merge(gid, A, gsize, false);
}

__kernel void sort_bitonic_desc(__global int* A) {
	int gid = get_global_id(0);
	int gsize = get_global_size(0);

	for (int i = 1; i < gsize/2; i*=2) {
		if (gid % (i*4) < i*2)
			bitonic_merge(gid, A, i*2, true);
		else if ((gid + i*2) % (i*4) < i*2)
			bitonic_merge(gid, A, i*2, false);
		barrier(CLK_GLOBAL_MEM_FENCE);
	}

	bitonic_merge(gid, A, gsize, true);
}

__kernel void standard_dev(__global const int* A, __global int* B, float avgVal, __local float* std_dev) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	B[gid] = pow((A[gid] - avgVal), 2);
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