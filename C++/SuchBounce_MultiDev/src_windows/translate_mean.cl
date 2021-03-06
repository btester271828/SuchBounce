#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void rmove(	__global double3 *r,			
			__global double3 *r_mean,
			__global int *groupID
		)
		{
			int x = get_global_id(0);
			r[x] -= r_mean[groupID[x]];
		}
