#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void interact(__global const int *k,
					__global const int *l,			
					__global double *q,					
					__global int *n,
					__global int *gespielt,
					__global int *opponent,
					__global double3 *r,
					__global double *stuff,
					__global int *genes,
					__global double *deltaq)
					
		{
			int x = get_global_id(0);			
			int a = k[x];
			int b = l[x];
			double q_control = 4.0e-10;				

			if ((opponent[a] == b) && (opponent[b] == a) && (opponent[a] != -1))// && (gespielt[a] != -1) && (gespielt[b] != -1))
			{	
				if (gespielt[a] == 0)
				{			
					if((genes[a] > genes[b])) // && genes[b] == -1)) // Geneticly determined charge swap polarity
					{					
						q[a] += q_control; q[b] -= q_control;
						deltaq[a] = q_control; deltaq[b] = -q_control;
						gespielt[a] = stuff[19]; gespielt[b] = -1.0*stuff[19];						
					}
					else
					{
						q[b] += q_control; q[a] -= q_control;
						deltaq[b] = q_control; deltaq[a] = -q_control;
						gespielt[b] =  stuff[19]; gespielt[a] = -1.0*stuff[19];
					}
					//opponent[a] = -1; opponent[b] = -1;									
				} 			
				else
				{
					if (gespielt[a] > 1) 
					{ 
						gespielt[a]--; gespielt[b]++; 
						deltaq[a] = 0.0; deltaq[b] = 0.0;
					}
					else
					{
						if (gespielt[b] > 1) 
						{ 
							gespielt[b]--; gespielt[a]++; 
							deltaq[a] = 0.0; deltaq[b] = 0.0;
						}
						else
						{	
							if(gespielt[a] == 1)
							{					
								q[a] -= q_control; q[b] += q_control;
								//deltaq[b] = q_control; deltaq[a] = -q_control;
							}	
							else
							{
								q[b] -= q_control; q[a] += q_control;
								//deltaq[a] = q_control; deltaq[b] = -q_control;
							}							
							gespielt[a] = 0; gespielt[b] = 0;
						}
					}
				}				
			}
			else
			{
				if (opponent[a] == -1) { deltaq[a] = 0.0; }
				if (opponent[b] == -1) { deltaq[b] = 0.0; }			
			}
		}					
					
					
					
