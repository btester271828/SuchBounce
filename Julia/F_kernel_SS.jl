import OpenCL
const cl = OpenCL

dev_list = cl.devices();
print("Select device for computations:\r\n")
for x = 1:length(dev_list)
	write(STDOUT,[string(x) ": "  string(dev_list[x])  "\r\n"])
end
n_choice = int64(readline(STDIN));

dev = dev_list[n_choice];
ctx = cl.Context(dev);
queue = cl.CmdQueue(ctx); 

const F_kernel = "
		__kernel void Fimp(__global const double *q,			
				 	__global const double *m,				  
				  	__global const double *I,
				  	__global const int *k,
					__global const int *l,			  	 				  	 
				 	__global const double *rad,	
				 	__global const double *stuff,			  
				 	__global double3 *r,
				 	__global double3 *v,
				 	__global double3 *w,
					__global double3 *rddp,
					__global double3 *oddp)
					
				  
		{ 			
			int x = get_global_id(0);
			int a = k[x] - 1;
			int b = l[x] - 1;
		    double G = stuff[6];
			double e0 = stuff[7];
			double d = distance(r[a],r[b]);
		 	double3 Runit = normalize(r[a] - r[b]);
			double3 wvec = w[a]*rad[a] + w[b]*rad[b];
			double3 vtemp = v[a] - v[b]; 
			double p = dot(vtemp,Runit);		
			double collisionflag = step(d,(rad[a]+rad[b]));			
			double c = d + (rad[a]+rad[b]);
			double F = (((m[a]*m[b]*G) - (q[a]*q[b]*e0))/(d*d) + (0.1666666)*stuff[2]*(pow(c,3)-2*(pow(rad[a],2) + pow(rad[b],2))*c)*(1/pow((c*c - (pow(rad[a],2) + pow(rad[b],2)) - (2*rad[a]*rad[b])),2)-1/pow((c*c - (pow(rad[a],2) + pow(rad[b],2)) + (2*rad[a]*rad[b])),2)));				
			double dF = ((-2*(m[a]*m[b]*G) + 2*(q[a]*q[b]*e0))/(d*d*d) + (0.1666666)*stuff[2]*((3*pow(c,2)-2*(pow(rad[a],2) + pow(rad[b],2)))*(1/pow((c*c - (pow(rad[a],2) + pow(rad[b],2)) - (2*rad[a]*rad[b])),2)-1/pow((c*c - (pow(rad[a],2) + pow(rad[b],2)) + (2*rad[a]*rad[b])),2)) - (4*pow(c,4)-8*(pow(rad[a],2) + pow(rad[b],2))*c*c)*(1/pow((c*c - (pow(rad[a],2) + pow(rad[b],2)) - (2*rad[a]*rad[b])),3)-1/pow((c*c - (pow(rad[a],2) + pow(rad[b],2)) + (2*rad[a]*rad[b])),3))))*p;		
		
			double3 v_rel = (vtemp - p*Runit) - cross(wvec,Runit);						
			double tempF = (F*stuff[0]+0.5*dF*stuff[0]*stuff[0]) - collisionflag*(stuff[8]*(rad[a]+rad[b]-d)*stuff[0] + 0.5*stuff[8]*p*stuff[0]*stuff[0]);			
			double Ff = collisionflag*step(0,length(v_rel))*(stuff[8]*(rad[a]+rad[b]-d)*stuff[0] + 0.5*stuff[8]*p*stuff[0]*stuff[0])*stuff[5];
			
			rddp[x] = tempF*Runit + Ff*normalize(v_rel);			
			oddp[x] = cross(Runit,Ff*normalize(v_rel));			
		}
"		

const r_kernel = " 
		__kernel void rstep(__global const double *stuff,					
					__global double3 *r,
					__global double3 *v,
					__global double3 *accel,
					__global double3 *alpha)
		{
			int x = get_global_id(0);		
			r[x] += (v[x]*stuff[0]);
			accel[x] = (0,0,0);
			alpha[x] = (0,0,0);
		}				
"
const v_kernel = " 
		__kernel void vstep(__global double3 *v,			
					__global double3 *w,
					__global double3 *accel,
					__global double3 *alpha)
		{
			int x = get_global_id(0);
			v[x] += 0.5*accel[x];
			w[x] += 0.5*alpha[x];			
		}				
"
const red_kernel = " 
		__kernel void red(__global double3 *rddp,
					__global double3 *oddp,
					__global double3 *accel,
					__global double3 *alpha,
					__global char *b,
					__global double *m,
					__global double *I,
					__global double *rad,
					__global int *n)
		{
			int x = get_global_id(0);			
			for (int c = 0; c < n[0]; c++)					
			{	
				accel[x] += b[c+n[0]*x]*rddp[c]/m[x];
				alpha[x] += b[c+n[0]*x]*b[c+n[0]*x]*oddp[c]*rad[x]/I[x];	
			}		
		}	
"
		
vstep = cl.Program(ctx, source=v_kernel) |> cl.build!
ker_v = cl.Kernel(vstep, "vstep");
rstep = cl.Program(ctx, source=r_kernel) |> cl.build!
ker_r = cl.Kernel(rstep, "rstep");
dynamics = cl.Program(ctx, source=F_kernel) |> cl.build!
ker_F = cl.Kernel(dynamics, "Fimp");
red = cl.Program(ctx, source=red_kernel) |> cl.build!
ker_S = cl.Kernel(red, "red");