__kernel void surface_coverage	(	
					__global double3 *r,
					__global double3 *orient,
					__global double *surface_coverage,
					__global double *n,
					__global double *rad,
					__global int *k,
					__global int *l,
					__global double *stuff
				)			
							
{
	int a = get_global_id(0);
	int n0 = get_global_size(0);
	int surface_blocks = stuff[12];
	int steps = sqrt(surface_blocks/2.0);

	for (int i = 0; i < 2*steps; i++)
	{
		for (int j = 0; j < steps; j++)
		{	
			surface_coverage[surface_blocks*a+i*steps+j] = 0.0;
		}
	}
	double d_orient = length(orient[a]);
	double phi_orient = acos(orient[a].z/d_orient);
	double theta_orient = atan(orient[a].y/orient[a].x);
	if (orient[a].x/(d_orient*sin(phi_orient)) < 0)
	{
		theta_orient += M_PI;
	}
	if (orient[a].x/(d_orient*sin(phi_orient)) > 0 && orient[a].y/sin(phi_orient) < 0)
	{
		theta_orient += 2.0*M_PI;
	}

	for (int b = 0; b < n0; b++)
	{
		if (b != a)
		{											
			double3 direction = r[a] - r[b];
			double d = length(direction);
			double phi_r = acos(direction.z/d);
			double theta_r = atan(direction.y/direction.x);
			if (direction.x/(d*sin(phi_r)) < 0)
			{
				theta_r += M_PI;
			}
			if (direction.x/(d*sin(phi_r)) > 0 && direction.y/(d*sin(phi_r)) < 0)
			{
				theta_r += 2.0*M_PI;
			}
			
			theta_r -= theta_orient;
			phi_r -= phi_orient;

			if (phi_r < 0.0) { phi_r += M_PI; }
			if (phi_r > M_PI) { phi_r -= M_PI; }
			if (theta_r < 0.0) { theta_r += 2.0*M_PI; }
			if (theta_r > 2.0*M_PI) { theta_r -= 2.0*M_PI; }
			
			for (int i = 0; i < 2*steps; i++)
			{
				for (int j = 0; j < steps; j++)
				{	
					if ((phi_r) >= (((double) (j))/((double) (1.0*steps)))*M_PI && (phi_r) < (((double) (j+1.0))/((double)  (1.0*steps)))*M_PI)
					{
						if ((theta_r) >= (((double) (i))/((double) (2.0*steps)))*2.0*M_PI && (theta_r) < (((double) (i+1.0))/((double) (2.0*steps)))*2.0*M_PI)
						{
							double dist = fmax((d-rad[a]-rad[b]),0);
							double activation = exp(-4.0*(dist));
							if( activation > surface_coverage[surface_blocks*a+i*steps+j]) { surface_coverage[surface_blocks*a+i*steps+j] = activation; }
							
						}
					}
				}
			}
		}
	}
}
					
	



