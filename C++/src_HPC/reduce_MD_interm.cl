#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void red_copy(__global double3 *rddp,
					__global double3 *oddp,
					__global double3 *accelsum,
					__global double3 *alphasum,
					__global const int *n,
					__global double *Vsum,
					__global double *Vpart,
					__global double *Internalsum,
					__global double *Ipart,
					__global const int *offset)
		{
			int x = get_global_id(0);
			int n_2 = n[2]; //get_global_size(0)*2/n[3];		
			int row = n_2*((offset[0]/n_2));
			int column = offset[0] - row;
			row += n_2*x;						
			Internalsum[row+column] = Ipart[128*x];
			Ipart[128*x] = 0;
			Vsum[row+column] = Vpart[128*x];
			Vpart[128*x] = 0;				
			accelsum[row+column] = rddp[128*x];
			rddp[128*x] = (0.0, 0.0, 0.0);					
			alphasum[row+column] = oddp[128*x];
			oddp[128*x] = (0.0, 0.0, 0.0);	
				
		}	
