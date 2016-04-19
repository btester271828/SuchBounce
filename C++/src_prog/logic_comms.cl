#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void comms(__global double3 *r,
					__global double *rad,		
					__global int *gespielt,						
					__global double *q,
					__global int *trig,
					__global int *n,
					__global double *ranges,
					__global int *opponent)
					
		{
			int x = get_global_id(0);		
			double signal = 0;
			
			if (gespielt[x] == 0)
			{
				opponent[x] = -1;
				double d_choice = rad[x]; //tactic #1
				double d = rad[x];
				int n_range = 0;
				int n_range_choice = 1;
						
				for (int i = 0; i < n[1]; i++)
				{										
					d = distance(r[x],r[i]);
					if (i != x && d < ranges[0] && d > (rad[x] + rad[i]))
					{
						n_range = 0;					
						for (int j = 0; j < n[1]; j++)
						{
							double d_ij = distance(r[j],r[i]);
							if (j != i && d_ij < ranges[0])
							{
								n_range++;					
							}					
						}
						//float optimality = 1.0;
						float optimality = 0.85*d_choice/d + 0.15*((float) (n_range)/(float) (n_range_choice));
						if ((optimality > 1.0 && gespielt[i] == 0))				
						{
							d_choice = d;
							n_range_choice = n_range;						
							opponent[x] = i;					
						}
					}
				}
			}
		}
			
