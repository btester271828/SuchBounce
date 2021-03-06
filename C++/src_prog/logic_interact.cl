#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void interact(__global const int *k,
					__global const int *l,			
					__global double *q,					
					__global int *n,
					__global int *gespielt,
					__global int *opponent,
					__global double3 *r,
					__global double *stuff,
					__global double *E_spent,
					__global double *E_lost,
					__global int *genes,
					__global int *rand)
					
		{
			int x = get_global_id(0);			
			int a = k[x];
			int b = l[x];
			double q_control = 5e-12;				

			if ((opponent[a] == b) && (opponent[b] == a) && (opponent[a] != -1))
			{
				E_spent[a] = 0.0;
				E_spent[b] = 0.0;
				E_lost[a] = 0.0;
				E_lost[b] = 0.0;
	
//				double E_interact = -0.5*stuff[7]*pow(q_control,2)/distance(r[a],r[b]);
//				if (gespielt[a] == 0)
//				{			
//					if(genes[a] == 1) 
//					{					
//						q[a] += q_control; q[b] -= q_control;
//					}
//					else
//					{
//						q[b] += q_control; q[a] -= q_control;
//					}					
//					E_spent[a] = E_interact;
//					E_spent[b] = E_interact;
//					gespielt[a] = 64; gespielt[b] = 64;
//				} 			
//				else
//				{
//					if (gespielt[a] > 1) { gespielt[a]--; gespielt[b]--; }
//					else
//					{	
//						if(genes[a] == 1)
//						{					
//							q[a] -= q_control; q[b] += q_control;
//						}	
//						else
//						{
//							q[b] -= q_control; q[a] += q_control;
//						}
//						E_lost[a] = E_interact;
//						E_lost[b] = E_interact;
//						gespielt[a] = 0; gespielt[b] = 0;
//					}
//				}				
//			}
//		}

				double E_interact = -0.5*stuff[7]*pow(q_control,2)/distance(r[a],r[b]);
				if (gespielt[a] == 0)
				{			
					//if((rand[a] > rand[b])) // Randomised charge swap polarity
					if((genes[a] == 1)) // && genes[b] == -1)) // Geneticly determined charge swap polarity
					{					
						q[a] += q_control; q[b] -= q_control;
						gespielt[a] = 64; gespielt[b] = -64;
					}
					else
					{
						q[b] += q_control; q[a] -= q_control;
						gespielt[b] = 64; gespielt[a] = -64;
					}					
					E_spent[a] = E_interact;
					E_spent[b] = E_interact;
					
				} 			
				else
				{
					if (gespielt[a] > 1) { gespielt[a]--; gespielt[b]++; }
					else
					{
						if (gespielt[b] > 1) { gespielt[b]--; gespielt[a]++; }
						else
						{	
							if(gespielt[a] == 1)
							{					
								q[a] -= q_control; q[b] += q_control;
							}	
							else
							{
								q[b] -= q_control; q[a] += q_control;
							}
							E_lost[a] = E_interact;
							E_lost[b] = E_interact;
							gespielt[a] = 0; gespielt[b] = 0;
						}
					}
				}				
			}
		}					
					
					
					
