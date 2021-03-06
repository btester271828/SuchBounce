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
					__global double *Ipart,
					__global double *pheremones,
					__global double *pheremonepart)
		{
			int x = get_global_id(0);			
							
			pheremones[x] = 0.995*pheremones[x] + pheremonepart[n[1]*x];	
			pheremonepart[n[1]*x] = 0.0;	
			Internal[x] = Ipart[n[1]*x];
			Ipart[n[1]*x] = 0.0;
			V[x] = 0.5*Vpart[n[1]*x];
			Vpart[n[1]*x] = 0.0;				
			accel[x] = rddp[n[1]*x]/m[x];
			rddp[n[1]*x] = (0.0, 0.0, 0.0);					
			alpha[x] = oddp[n[1]*x]/I[x];
			oddp[n[1]*x] = (0.0, 0.0, 0.0);					
		}	
