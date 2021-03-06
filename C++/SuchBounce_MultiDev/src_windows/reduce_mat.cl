#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void red(__global double3 *rddp,
					__global double3 *oddp,
					__global double3 *accel,
					__global double3 *alpha,
					__global const double *m,
					__global const double *I,
					__global const double *rad,
					__global const int *n,
					__global double *V,
					__global double *Vpart,
					__global double *Internal,
					__global double *Ipart)
		{
			int x = get_global_id(0);			
									
			Internal[x] = Ipart[n[1]*x];
			Ipart[n[1]*x] = 0.0;
			V[x] = 0.5*Vpart[n[1]*x];
			Vpart[n[1]*x] = 0.0;	
			
			accel[x] = rddp[n[1]*x]/m[x];
			//rddp[n[1]*x] = (0.0, 0.0, 0.0);
			rddp[n[1]*x].x = 0.0;
			rddp[n[1]*x].y = 0.0;
			rddp[n[1]*x].z = 0.0;
			//rddp[n[1]*x] -= rddp[n[1]*x];					
			alpha[x] = oddp[n[1]*x]/I[x];
			//oddp[n[1]*x] = (0.0, 0.0, 0.0);
			oddp[n[1]*x].x = 0.0;
			oddp[n[1]*x].y = 0.0;
			oddp[n[1]*x].z = 0.0;
			//oddp[n[1]*x] -= oddp[n[1]*x];					
		}	
