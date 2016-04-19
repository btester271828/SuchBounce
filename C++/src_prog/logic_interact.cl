#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void interact(__global const int *k,
					__global const int *l,			
					__global double *q,
					__global int *n,
					__global int *gespielt,
					__global int *opponent)
					
		{
			int x = get_global_id(0);			
			int a = k[x];
			int b = l[x];
			double q_control = 1e-10;
			if (opponent[a] == b && opponent[b] == a && opponent[a] != -1)
			{
				if (gespielt[a] == 0)
				{			
					q[a] += q_control; q[b] -= q_control;
					gespielt[a] = 16; gespielt[b] = 16;
				} 			
				else
				{
					if (gespielt[a] > 1) { gespielt[a]--; gespielt[b]--; }
					else
					{	
						q[a] -= q_control; q[b] += q_control;
						gespielt[a] = 0; gespielt[b] = 0;
					}
				}				
			}
		}					
					
