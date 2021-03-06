#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void comms(__global double3 *r,
					__global const double *rad,		
					__global int *gespielt,						
					__global double *q,
					__global int *trig,
					__global int *n,
					__global const double *ranges,
					__global int *opponent,
					__global int *genes)
					
		{
			int x = get_global_id(0);		
			double signal = 0;
			
			if (gespielt[x] == 0 && genes[x] != 0)
			{
				opponent[x] = -1;
				double d_choice = ranges[0]; //rad[x]; // //CHANGE TACTIC HERE AND BELOW
				int n_range = 0;
				int n_range_choice = 1;
				//int test_choice = 1;
						
				for (int i = 0; i < n[1]; i++)
				{										
					double d = distance(r[x],r[i]);
					//if ((i != x) && (d < ranges[0]) && d > (rad[x] + rad[i]))// random
					if ((i != x) && (d < ranges[0]) && d > (rad[x] + rad[i]) && (genes[x] != genes[i]) && genes[i] != 0)//genetic
					{
										
						double optimality = d_choice/d; // + 0.0*((float) (n_range)/(float) (n_range_choice)); //CHANGE TACTIC HERE AND ABOVE
						if ((optimality > 1.0) && (gespielt[i] == 0))				
						{
							d_choice = d;
							n_range_choice = n_range;						
							opponent[x] = i;
							//test_choice = i;					
						}
					}
				}
			}
		}
			
