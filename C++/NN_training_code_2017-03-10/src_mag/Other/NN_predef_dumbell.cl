__kernel void neural_net(	
				__global double* weights,
				__global double* activation_t0,
				__global double* activation_t1,
				__global double3* mu,								
				__global double* stuff,
				__global double3* positions
			)
{
	int x = get_global_id(0);
	int net_size = stuff[11];
	int NN_outputs = stuff[13];	
	
	
	double x_flag = stuff[3];
	if (x_flag == 1)
	{
		
		double theta = atan(positions[x].y/positions[x].x);
		mu[x].x = sign(positions[x].x)*maxmag(-sin(2*theta)*cos(theta),0.0);
		mu[x].y = sign(positions[x].x)*maxmag(cos(2*theta)*cos(theta),0.0);
		mu[x].z = 0.0;		
	}
	else
	{
		double theta = atan(positions[x].z/positions[x].x);
		mu[x].x = sign(positions[x].x)*maxmag(-sin(2*theta)*cos(theta),0.0);		
		mu[x].y = 0.0;		
		mu[x].z = sign(positions[x].x)*maxmag(cos(2*theta)*cos(theta),0.0);
	
	}
}
			
		
							
							


