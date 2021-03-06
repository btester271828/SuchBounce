#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void vstep(__global double3 *v,					
					__global double3 *accel,
					__global double *F_track)
					
		{
			int x = get_global_id(0);
			v[x] += 0.5*accel[x];	
			F_track[3] =  maxmag(F_track[3],length(v[x]));							
		}
