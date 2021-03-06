#include "../Settings.h"

//Auto-stuff
double stuff[18] = { 0.0 }; 						// container for various simulation parameters 
double t_step = 0;
const int n_el = (0.5)*n*(n - 1); 					// number of distinct particle interaction matrix elements
const int n_frames = (max_time * 64 / warp);				// Calculated interval between data samples for output	



template <typename T>
std::vector<T> &operator+=(std::vector<T> &A, const std::vector<T> &B)	// defines "+=" for vectors as concatenation
{
    A.reserve( A.size() + B.size() );                	
    A.insert( A.end(), B.begin(), B.end() );         	
    return A;                                        	
}
int string2array(std::string str, double* arr, int length)			// converts csv string to double array
{
	std::stringstream streamy(str);
	for (int i = 0; i < length; i++)
	{
		std::string tempstr;
		std::getline(streamy, tempstr, ',');
		arr[i] = std::stod(tempstr);
	}
	
}


std::string arraytostring(double a[n][4], int n)				// converts n-by-4 array to string format [a,b,c,d;e,f,g,h]
{	
	std::string out = "[";
	for (int j = 0; j < n; j++)
	{
		for (int k = 0; k < 3; k++)
		{	
			char temp[25];
			sprintf(temp, "%+.13e", a[j][k]);
			out.append(std::string(temp));
			if (k != 2){out.append(", "); }		
		}
		if (j != (n - 1)){ out.append("; "); }
	}
	out.append("]\n");
	return out;
}

std::string arraytostring(double a[n][net_size], int n)				
{	
	std::string out = "[";
	for (int j = 0; j < n; j++)
	{
		for (int k = 0; k < net_size; k++)
		{	
			char temp[25];
			sprintf(temp, "%+.13e", a[j][k]);
			out.append(std::string(temp));
			if (k != net_size - 1){out.append(", "); }		
		}
		if (j != (n - 1)){ out.append("; "); }
	}
	out.append("]\n");
	return out;
}


std::string arraytostring(double a[n], int n)
{
	std::string out = "[";
	for (int j = 0; j < n; j++)
	{
			char temp[25];
			sprintf(temp, "%+.13e", a[j]);
			out.append(std::string(temp));
			if (j != (n-1)){ out.append(", "); }
	}
	out.append("]\n");
	return out;
}

cl::Kernel kernel_init(std::string file, std::string ker_func, cl::Context ctx, std::vector<cl::Device> ctxdev, std::ofstream &logfile) // Compiles, Builds and establishes OpenCL kernel from .cl source file
{
	logfile << "File: " << file << ":\n";
	std::stringstream tempstr;
	std::ifstream programFile(file);
	std::string programString(std::istreambuf_iterator<char>(programFile), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(programString.c_str(), programString.length()+1));		
	cl::Program program(ctx, source);			
	try{ program.build(ctxdev, "-cl-finite-math-only"); }
	catch (cl::Error e)
	{
		std::cout << "File " << file << ": "
		<< e.what() << ";" << e.err() << "\n";
		
	}
	tempstr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(ctxdev[0]);	
	logfile << tempstr.str().substr(0, tempstr.str().size()-1) << "\n";	
	logfile.flush();
		
	cl::Kernel kernel(program, ker_func.c_str());
	

	return kernel;
}


cl::Buffer buffer_init(cl::Context ctx, ::size_t size)					// Initialises empty OpenCL buffer
{
	cl::Buffer new_buff(ctx, CL_MEM_READ_WRITE, size);
	return new_buff;
}

cl::Buffer buffer_init(cl::Context ctx, ::size_t size, int a)				// Initialises OpenCL buffer populated with data from variable "a"
{
	cl::Buffer new_buff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, &a);
	return new_buff;
}


int main() 
{
	auto r = new double[n][4];
	auto v = new double[n][4];
	auto w = new double[n][4];
	//auto mu = new double[n][4];	
	auto q = new double[n];
	auto m = new double[n];
	auto rad = new double[n];
	auto groups = new int[n*n];
	auto joined = new short[n][n];
	auto weights = new double[net_size][net_size];
	std::string path;
	std::string root = "../Setup";

	path = root + "/";
	std::ifstream r_in(path + "r.vec");
	std::ifstream v_in(path + "v.vec");
	std::ifstream w_in(path + "w.vec");
	//std::ifstream mu_in(path + "mu.vec");
	std::ifstream rad_in(path + "rad.vec");	
	std::ifstream q_in(path + "q.vec");
	std::ifstream m_in(path + "m.vec");

	std::ifstream weight_in(path + "weights.vec");
	
	for (int i = 0; i < n; i++)
	{
		std::string r_str = "";
		std::string v_str = "";
		std::string w_str = "";
		//std::string mu_str = "";		
		std::string q_str = "";
		std::string m_str = "";
		std::string rad_str = "";				
		double r_temp[4] = {0.0, 0.0, 0.0, 0.0};
		double v_temp[4] = {0.0, 0.0, 0.0, 0.0};
		double w_temp[4] = {0.0, 0.0, 0.0, 0.0};
		double mu_temp[4] = {0.0, 0.0, 0.0, 0.0};
		double q_temp = 0.0;
		double rad_temp = 0.0;
		double m_temp = 0.0;		

		std::getline(r_in,r_str);
		std::getline(v_in,v_str);
		std::getline(w_in,w_str);
		//std::getline(mu_in,mu_str);
		std::getline(q_in,q_str);
		std::getline(m_in,m_str);
		std::getline(rad_in,rad_str);
		
		string2array(r_str,r_temp,3);
		string2array(v_str,v_temp,3);
		string2array(w_str,w_temp,3);
		//string2array(mu_str,mu_temp,3);
		
		q_temp = std::stod(q_str);		
		m_temp = std::stod(m_str);		
		rad_temp = std::stod(rad_str);	
		
		q[i] = q_temp;
		rad[i] = rad_temp;
		m[i] = m_temp;
		
		for (int j = 0; j < 4; j++)
		{		
			r[i][j] = r_temp[j];
			v[i][j] = v_temp[j];
			w[i][j] = w_temp[j];
			//mu[i][j] = mu_temp[j];		
		}
	}
	
//	for (int i = 0; i < net_size; i++)
//	{
//		std::string weight_str = "";						
//		double weight_temp[net_size] = {0.0};			
//		std::getline(weight_in, weight_str);					
//		string2array(weight_str,weight_temp,net_size);	
//		for (int j = 0; j < net_size; j++)
//		{				
//			weights[i][j] = weight_temp[j];
//		}					
//	}

		
	stuff[0] = settings[1];
	stuff[1] = settings[3];
	stuff[2] = settings[4];
	
	stuff[4] = settings[6];
	stuff[5] = settings[7];
	stuff[6] = 6.67384e-11;
	stuff[7] = pow((4 * 3.141592654*8.85419e-12), -1); // electrostatic force constant
	stuff[8] = settings[8];
	stuff[9] = settings[2];
	stuff[10] = settings[1];
	stuff[11] = net_size;
	stuff[12] = surface_blocks;
	stuff[13] = NN_outputs;
	stuff[14] = settings[10];
	stuff[15] = settings[11];
	stuff[16] = settings[12]; 
	stuff[17] = settings[13];
	if (stuff[0] < 0)
	{	
		stuff[4] = 0.0;
		stuff[5] = 0.0;
		std::cout << "Reverse time integration...friction constants set to zero and time-reversed setae drag used\n";
	}

	if (n_frames > max_time/fabs(stuff[9]))
	{	
		std::cout << "Insufficient frames for specified warp; frame intervals will be wierd\r\n";
		std::cout << n_frames << "; " << (max_time/stuff[9]) << "\n";
	}

	int framecount = 0;
	int tempcount = 0;
	auto E_temp = new double[n];
	auto surf_temp = new double[n][net_size];//[surface_blocks];
	auto I = new double[n];
	auto l1 = new int[n_el];
	auto l2 = new int[n_el];
	auto l3 = new int[(n/n_block[0])*(n/n_block[0]+1)/2];
	auto l4 = new int[(n/n_block[0])*(n/n_block[0]+1)/2];
	auto l1temp = new int[n_block[1]*n_block[2]];
	auto l2temp = new int[n_block[1]*n_block[2]];
	auto l1_block0 = new int[n_block[1]*n_block[2]];
	auto l2_block0 = new int[n_block[1]*n_block[2]];
	
	
	int n0[6] = { n_el, n, n , n, n_block[0], n};

	for (int x=0; x<n; x++)
	{
		I[x] = (2*m[x]*(pow(rad[x],2))/5);
		
	}
	
	for (int x=1; x<n_block[0]; x++)
	{
		for (int y=0; y<x; y++)	
		{	
			int i = (0.5*x*(x-1)+y);			
			l1temp[i] = x;
			l2temp[i] = y;
			l1_block0[i] = x;
			l2_block0[i] = y;			
		}
		
	}
	

	int count[2] = { 0 };
	int offsets_0[(n/n_block[0])*((n/n_block[0])+1)/2]; 
	int offsets_1[int (pow(n,2)/pow(n_block[0],2))];
	offsets_0[0] = 0;
	for (int p = 0; p<(n/n_block[0]); p++)
	{
		for (int q = 0; q<p; q++)
		{
			offsets_0[count[1]+1] = offsets_0[count[1]] + n_block[0]*n_block[0];
			l3[count[1]] = p;
			l4[count[1]] = q;
			count[1]++;
        		for (int s = 0; s<n_block[0]; s++)
			{
            			for (int t = 0; t<n_block[0]; t++)
				{
                			l1[count[0]] = p*n_block[0] + t;
                			l2[count[0]] = q*n_block[0] + (s + t - n_block[0]*floor((s+t)/n_block[0]));
                			count[0]++;
				}
			}
		}
		
		offsets_0[count[1]+1] = offsets_0[count[1]] + n_block[1]*n_block[2];		
		l3[count[1]] = p; l4[count[1]] = p;
		count[1]++;
		for (int i=0; i<(n_block[1]*n_block[2]); i++)
		{    		
							
			l1[count[0]] = l1_block0[i] + n_block[0]*p;
    			l2[count[0]] = l2_block0[i] + n_block[0]*p;
    			count[0]++;
		}
		for (int u = 0; u<(n/n_block[0]); u++)
		{
			offsets_1[(n/n_block[0])*p+u] = n*p+u;

		}
    		
	}
	
	//std::cout << "Generated interaction maps...\n";
	delete [] l1temp;
	delete [] l2temp; 
	delete [] l1_block0;
	delete [] l2_block0;
	

	// OpenCL Context Sorcery
	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> platformDevices, allDevices, conDev;
	std::string device_name;
	cl::Device DevChoice;
	int nDev;
	cl::Platform::get(&platforms);
	platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);
	
		
	std::cout << "Please Choose Device:\n\n";
	for (int i = 0; i < platformDevices.size(); i++)
	{
		device_name = platformDevices[i].getInfo<CL_DEVICE_NAME>();
		std::cout << "Device " << i << ": "
		<< device_name
		<< std::endl;
	}
	//std::cin >> nDev;
	nDev = 0;
	std::vector<cl::Device> ctxDevices = { platformDevices[nDev] };
	cl::Context ctx(ctxDevices[0]);
	cl::CommandQueue queue(ctx, ctxDevices[0]);

	//std::cout << "1\n"; std::cout.flush();

	root = "../Outputs";	
	path = root + "/";
	std::ofstream OpenCL_log(path + "OpenCL_log.txt", std::ios::out);
	
		
	// Build Kernels	
	cl::Kernel ker_F;
	if (stuff[0] < 0) { ker_F = kernel_init("F_Hybrid_mat_reversed.cl", "Fimp", ctx, ctxDevices, OpenCL_log); }	
	else { ker_F = kernel_init("F_Hybrid_mat.cl", "Fimp", ctx, ctxDevices, OpenCL_log); }
	
	cl::Kernel ker_T = kernel_init("kinetic.cl", "Tstep", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_r = kernel_init("position.cl", "rstep", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_S = kernel_init("reduce_mat.cl", "red", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_Sp = kernel_init("reduce_pairwise.cl", "red_pair", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_S_interm = kernel_init("reduce_mat_interm.cl", "red_copy", ctx, ctxDevices, OpenCL_log);	
	cl::Kernel ker_t = kernel_init("translate.cl", "rmove", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_t0 = kernel_init("translate_0.cl", "rmove0", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_t_mean = kernel_init("translate_mean.cl", "rmove", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_v_0 = kernel_init("velocity.cl", "vstep", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_v_1 = kernel_init("velocity.cl", "vstep", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_0_0 = kernel_init("zero.cl", "zeroer", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_0_1 = kernel_init("zero.cl", "zeroer", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_0_2 = kernel_init("zero_vec.cl", "zeroer", ctx, ctxDevices, OpenCL_log);
	
	cl::Kernel ker_scale = kernel_init("time_scaler.cl", "Scale", ctx, ctxDevices, OpenCL_log);		
	

	//std::cout << "2\n"; std::cout.flush();

	// Assign Buffers
	double zerotemp_4[4] = { 0.0, 0.0, 0.0, 0.0 }; 
	::size_t vecsize = ::size_t(4*n*8);

	cl::Buffer nbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(n0), n0);	
	cl::Buffer l2buff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n_el), l2);
	cl::Buffer l1buff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n_el), l1);
	
	cl::Buffer cbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(8*n), q);
	cl::Buffer mbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(8*n), m);
	cl::Buffer radbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(8*n), rad);
	cl::Buffer Ibuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(8*n), I);
	cl::Buffer tbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(stuff), stuff);
	cl::Buffer Ftmp(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ::size_t (32), zerotemp_4);

	cl::Buffer rbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, r);
	cl::Buffer r_obs_buff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, r);
	cl::Buffer vbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, v);
	cl::Buffer wbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, w);	

	//std::cout << "3\n"; std::cout.flush();

	std::vector<cl::Buffer> vincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0]*4)) };
	std::vector<cl::Buffer> wincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0]*4)) };
	std::vector<cl::Buffer> offset_buff_0 = { buffer_init(ctx,::size_t (4), 0) };
	std::vector<cl::Buffer> offset_buff_1 = { buffer_init(ctx,::size_t (4), 0) };
	std::vector<cl::Buffer> Vincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0])) };
	std::vector<cl::Buffer> Intincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0])) };
	//std::cout << "Attempting to split " << (pow(n,2)/pow(n_block[0],2)) << " ways...\n";	
	//std::cout << "4\n"; std::cout.flush();
	for (int i = 1; i<(pow(n,2)/pow(n_block[0],2)); i++)
	{
		vincbuff += { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0]*4)) };
		wincbuff += { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0]*4)) };
		Vincbuff += { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0])) };
		Intincbuff += { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0])) };		
	}
	
	for (int i = 1; i<0.5*(n/n_block[0])*((n/n_block[0])+1); i++)
	{
		offset_buff_0 += { buffer_init(ctx, ::size_t (4), offsets_0[i]) };
		//ker_F += { kernel_init("F_Hybrid_mat.cl", "Fimp", ctx, ctxDevices) };		
		//std::cout << offsets_0[i] << "; ";
		
	}
	
	for (int i = 1; i<pow(n,2)/pow(n_block[0],2); i++)
	{
		offset_buff_1 += { buffer_init(ctx, ::size_t (4), offsets_1[i]) };
		//std::cout << offsets_1[i] << "; ";
		
	}
	//std::cout << "5\n"; std::cout.flush();
	std::cout << "Split particles into " << vincbuff.size() << " blocks\n";

	
	cl::Buffer accelbuff(ctx, CL_MEM_READ_WRITE, vecsize);
	cl::Buffer alphabuff(ctx, CL_MEM_READ_WRITE, vecsize);	

	cl::Buffer accelsumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n*n/n_block[0]*4));
	cl::Buffer alphasumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n*n/n_block[0]*4));
	cl::Buffer Vsumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n*n/n_block[0]));
	cl::Buffer Intsumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n*n/n_block[0]));

	cl::Buffer Vbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n));
	cl::Buffer Intbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n));
	cl::Buffer Tvbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n));
	cl::Buffer Twbuff(ctx, CL_MEM_READ_WRITE, ::size_t (8*n));	
	
	// Open output streams
	root = "../Outputs";	
	path = root + "/";
	std::ofstream r_tracker(path + "Particle_tracks.dat", std::ios::out);
	std::ofstream Tv_tracker(path + "T_v_tracks.dat", std::ios::out);
	std::ofstream Tw_tracker(path + "T_w_tracks.dat", std::ios::out);
	std::ofstream E_tracker(path + "E_int_tracks.dat", std::ios::out);
	std::ofstream V_tracker(path + "V_tracks.dat", std::ios::out);	
	std::ofstream w_tracker(path + "w_tracks.dat", std::ios::out);
	std::ofstream stuff_tracker(path + "t_steps.dat", std::ios::out);
	
	std::ofstream r_final(path + "r_final.dat", std::ios::out);
	std::ofstream v_final(path + "v_final.dat", std::ios::out);
	std::ofstream w_final(path + "w_final.dat", std::ios::out);
	
	//Set Kernel Arguments	

	ker_F.setArg(0, cbuff);
	ker_F.setArg(1, mbuff);
	ker_F.setArg(2, Ibuff);
	ker_F.setArg(3, l1buff);
	ker_F.setArg(4, l2buff);
	ker_F.setArg(5, radbuff);
	ker_F.setArg(6, tbuff);
	ker_F.setArg(7, rbuff);
	ker_F.setArg(8, vbuff);
	ker_F.setArg(9, wbuff);
	ker_F.setArg(10, wbuff);
	int a = l3[0];
	int b = l4[0];	
	ker_F.setArg(11, vincbuff[a*(n/n_block[0])+b]);
	ker_F.setArg(12, wincbuff[a*(n/n_block[0])+b]);
	ker_F.setArg(15, Vincbuff[a*(n/n_block[0])+b]);
	ker_F.setArg(16, Intincbuff[a*(n/n_block[0])+b]);
	ker_F.setArg(13, vincbuff[b*(n/n_block[0])+a]);
	ker_F.setArg(14, wincbuff[b*(n/n_block[0])+a]);
	ker_F.setArg(17, Vincbuff[b*(n/n_block[0])+a]);
	ker_F.setArg(18, Intincbuff[b*(n/n_block[0])+a]);			
	ker_F.setArg(21, offset_buff_0[0]);
	ker_F.setArg(19, Ftmp);
	ker_F.setArg(20, nbuff);
		
	
	ker_S.setArg(0, vincbuff[0]);
	ker_S.setArg(1, wincbuff[0]);
	ker_S.setArg(2, accelbuff);
	ker_S.setArg(3, alphabuff);
	ker_S.setArg(4, mbuff);
	ker_S.setArg(5, Ibuff);
	ker_S.setArg(6, radbuff);
	ker_S.setArg(7, nbuff);
	ker_S.setArg(8, Vbuff);
	ker_S.setArg(9, Vincbuff[0]);
	ker_S.setArg(10, Intbuff);
	ker_S.setArg(11, Intincbuff[0]);

	ker_S_interm.setArg(2, accelsumbuff);
	ker_S_interm.setArg(3, alphasumbuff);
	ker_S_interm.setArg(4, nbuff);
	ker_S_interm.setArg(5, Vsumbuff);
	ker_S_interm.setArg(7, Intsumbuff);


	ker_Sp.setArg(0, vincbuff[0]);
	ker_Sp.setArg(1, wincbuff[0]);	
	ker_Sp.setArg(2, nbuff);	
	ker_Sp.setArg(3, Vincbuff[0]);
	ker_Sp.setArg(4, Intincbuff[0]);

	ker_0_0.setArg(0, accelbuff);
	ker_0_0.setArg(1, Vbuff);
	ker_0_1.setArg(0, alphabuff);
	ker_0_1.setArg(1, Intbuff);

	ker_0_2.setArg(0, wbuff);	

	ker_v_0.setArg(0, vbuff);
	ker_v_0.setArg(1, accelbuff);
	ker_v_1.setArg(0, wbuff);
	ker_v_1.setArg(1, alphabuff);

	ker_r.setArg(0, tbuff);
	ker_r.setArg(1, rbuff);
	ker_r.setArg(2, vbuff);	
	
	ker_t.setArg(0,rbuff);

	ker_t0.setArg(0,rbuff);

	ker_T.setArg(0, vbuff);
	ker_T.setArg(1, wbuff);
	ker_T.setArg(2, Tvbuff);
	ker_T.setArg(3, Twbuff);
	ker_T.setArg(4, mbuff);
	ker_T.setArg(5, Ibuff);
	
	cl::NDRange offset(0);
	cl::NDRange gsize1(n);
	cl::NDRange gsize1m(n-1);
	cl::NDRange unitsize(1);
	std::vector<cl::NDRange> gsize2 = { ::size_t(n_el), ::size_t(n_block[1]*n_block[2]), ::size_t(n_block[0]*n_block[0]) };
	cl::NDRange local_size(workgroup_size);
	std::cout << "Kernels & Buffers set...";

		
	
	// Initialise timestep scaler	
	double v_norm[n] = { 0.0 };

	double v_max = 0;


	for (int z = 0; z < n; z++)
	{		
		v_norm[z] = sqrt(pow(v[z][0], 2) + pow(v[z][1], 2) + pow(v[z][2], 2));
		if (z > 0){ v_max = std::max(v_norm[z], v_norm[z - 1]); }		
	}
	
	double F1[4] = { 0.0 };
	
		
	for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
	{
		int a = l3[i];
		int b = l4[i];
		int waitcount = 0;
		//std::cout << offsets_0[i] << "\n";
		queue.flush();
		ker_F.setArg(11, vincbuff[a*(n/n_block[0])+b]);
		ker_F.setArg(12, wincbuff[a*(n/n_block[0])+b]);
		ker_F.setArg(15, Vincbuff[a*(n/n_block[0])+b]);
		ker_F.setArg(16, Intincbuff[a*(n/n_block[0])+b]);
		ker_F.setArg(13, vincbuff[b*(n/n_block[0])+a]);
		ker_F.setArg(14, wincbuff[b*(n/n_block[0])+a]);
		ker_F.setArg(17, Vincbuff[b*(n/n_block[0])+a]);
		ker_F.setArg(18, Intincbuff[b*(n/n_block[0])+a]);			
		ker_F.setArg(21, offset_buff_0[i]);		
						
		if (a == b) {queue.enqueueNDRangeKernel(ker_F, offset, gsize2[1], local_size); } 		// Compute force
		else { queue.enqueueNDRangeKernel(ker_F, offset, gsize2[2], local_size); } 							
	}				
	
	queue.enqueueReadBuffer(Ftmp, CL_TRUE, ::size_t (0), ::size_t (32), &F1);
	
	double scaleset[4] = { F1[1], F1[1], v_max, v_max };//	std::cout << F1[2] << "," << v_max << "\n";	
	
	
	cl::Buffer Fbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ::size_t(32), scaleset);
	ker_F.setArg(19, Fbuff);
	ker_v_0.setArg(2, Fbuff);
	ker_v_1.setArg(2, Ftmp);
	ker_scale.setArg(0, tbuff);
	ker_scale.setArg(1, Fbuff);
	
		

	// Initialise simulation
	double t_now = 0;	
	double t_last = 0;
	if (stuff[0] < 0) { t_now = max_time; t_last = max_time;}
	short int prog = 0;
	//double r_mean[4] = {0.0, 0.0, 0.0, 0.0};
	double r_mean[n][4];
	count[0] = 0;
	std::string tempstring;

	queue.enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
	queue.enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things	
		
	tempstring = arraytostring(r,n);
	r_tracker << tempstring;
	
	std::cout << "..simulation started.\n";
	//for (int i=0; i<100; i++){std::cout << "-";}
	std::cout << "\n";

//	std::cout << "0%   10   20   30   40   50   60   70   80   90   100%\n";
//	std::cout << "|----|----|----|----|----|----|----|----|----|----|\n";
//		     
	//clock_t t0 = clock();
	//clock_t t_temp = t0;
	
	time_t t0 = time(NULL);
	
	// queue.enqueueNDRangeKernel(ker_joints, offset, gsize2[0], local_size);
	// queue.enqueueReadBuffer(jointbuff, CL_TRUE, ::size_t (0), ::size_t(2*n*n), joined);
	//std::cout << net_group_find(joined, groups)	 << "\n";
	// for(int i = 0; i < n; i++)
	// {
		// double temp[n];
		// for (int j = 0; j < n; j++) {temp[j] = groups[j+n*i];}
		// tempstring = arraytostring(temp,n);
		// groups_track << tempstring;
	// }

	for(int init_x=0; init_x<t_test; init_x ++)
	{
		//std::cout << init_x << "\n";		

		
		queue.enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick

		queue.enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size);	// Rotational Kick

		queue.enqueueNDRangeKernel(ker_T, offset, gsize1, local_size); 		// Evaluate Kinetic Energy
		queue.finish();		
		

		if (( t_now == 0 || fabs(t_now - t_last) >= (1.0 / 64.0)*warp) && framecount < n_frames)
		{
			framecount++;
			if (floor(100 * framecount / n_frames) > 4+floor(prog)){prog = floor(100 * framecount / n_frames); std::cout << prog << "%\n";}	
//			if (floor(100 * framecount / n_frames) > 1+floor(prog))
//			{
//				prog = floor(100 * framecount / n_frames); 
//				std::cout << "*";
//				std::cout.flush();
//			}		
			
			
			queue.enqueueReadBuffer(rbuff, CL_TRUE, ::size_t (0), vecsize, r);
			tempstring = arraytostring(r,n);
			r_tracker << tempstring;

			queue.enqueueReadBuffer(Vbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			V_tracker << tempstring;

			queue.enqueueReadBuffer(Intbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
//			tempstring = arraytostring(stuff, 11);
			E_tracker << tempstring;

			queue.enqueueReadBuffer(Tvbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tv_tracker << tempstring;
		
			queue.enqueueReadBuffer(Twbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tw_tracker << tempstring;

			tempstring = arraytostring(stuff,11);
			stuff_tracker << tempstring;

			//queue.enqueueReadBuffer(mubuff, CL_TRUE, ::size_t (0), vecsize, mu);
			//tempstring = arraytostring(mu, n);
			//mu_tracker << tempstring;

			//queue.enqueueReadBuffer(coveragebuff, CL_TRUE, ::size_t (0), ::size_t(8*50*n), surf_temp);
			//tempstring = arraytostring(surf_temp, n);
			//coverage_tracker << tempstring; 	
		
		
			
//			queue.enqueueReadBuffer(accelbuff, CL_TRUE, ::size_t (0), vecsize, w);
//			tempstring = arraytostring(w, n);
//			w_tracker << tempstring;
			

			t_last = t_now;
		

		}
		queue.enqueueReadBuffer(tbuff, CL_TRUE, ::size_t(0), sizeof(stuff), stuff);
		t_now += stuff[0];
		queue.enqueueNDRangeKernel(ker_scale,offset,local_size,local_size); 	// Set new time step
		
		queue.enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
		queue.enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things
		
	
		queue.enqueueNDRangeKernel(ker_r, offset, gsize1, local_size); 		// Drift	


		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
		{
			int a = l3[i];
			int b = l4[i];
			int waitcount = 0;
			//std::cout << offsets_0[i] << "\n";
			queue.flush();
			ker_F.setArg(11, vincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(12, wincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(15, Vincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(16, Intincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(13, vincbuff[b*(n/n_block[0])+a]);
			ker_F.setArg(14, wincbuff[b*(n/n_block[0])+a]);
			ker_F.setArg(17, Vincbuff[b*(n/n_block[0])+a]);
			ker_F.setArg(18, Intincbuff[b*(n/n_block[0])+a]);			
			ker_F.setArg(21, offset_buff_0[i]);		
							
			if (a == b) {queue.enqueueNDRangeKernel(ker_F, offset, gsize2[1], local_size); } 		// Compute force
			else { queue.enqueueNDRangeKernel(ker_F, offset, gsize2[2], local_size); } 
				
			
		}
		//std::cout << "b";
		n0[3] = n_block[0];
		n0[2] = n_block[0];
		n0[1] = n_block[0];

		queue.enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);
		
		for (int a = 0; a < (n/n_block[0]); a++)
		{
			for (int b = 0; b < (n/n_block[0]); b++)
			{		
				//int a = l3[i];
				//int b = l4[i];								
				
				n0[2] = n0[1];
				queue.flush();						
				ker_Sp.setArg(0, vincbuff[a*(n/n_block[0])+b]);
				ker_Sp.setArg(1, wincbuff[a*(n/n_block[0])+b]);
				ker_Sp.setArg(3, Vincbuff[a*(n/n_block[0])+b]);
				ker_Sp.setArg(4, Intincbuff[a*(n/n_block[0])+b]);
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue.enqueueNDRangeKernel(ker_Sp, offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}				
							
			}				
		}
		//std::cout << "c";
		
		n0[1] = n;
		n0[2] = n/n_block[0];
		n0[3] = n;
		queue.enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
		{	

			int a = l3[i];
			int b = l4[i];				
			cl::NDRange gsizeRed(n_block[0]);
	
			if (a != b)
			{
			
				queue.flush();
				ker_S_interm.setArg(0, vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(1, wincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(6, Vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(8, Intincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(9, offset_buff_1[a*(n/n_block[0])+b]);			
						
				queue.enqueueNDRangeKernel(ker_S_interm, offset, gsizeRed, local_size);
						
				queue.flush();
				ker_S_interm.setArg(0, vincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(1, wincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(6, Vincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(8, Intincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(9, offset_buff_1[b*(n/n_block[0])+a]);			
					
				queue.enqueueNDRangeKernel(ker_S_interm, offset, gsizeRed, local_size);
							
			}
			else
			{
				queue.flush();
				ker_S_interm.setArg(0, vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(1, wincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(6, Vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(8, Intincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(9, offset_buff_1[a*(n/n_block[0])+b]);			
						
				queue.enqueueNDRangeKernel(ker_S_interm, offset, gsizeRed, local_size);
				
			}
		}
		//std::cout << "d";

		ker_Sp.setArg(0, accelsumbuff);
		ker_Sp.setArg(1, alphasumbuff);
		ker_Sp.setArg(3, Vsumbuff);
		ker_Sp.setArg(4, Intsumbuff);
		ker_S.setArg(0, accelsumbuff);
		ker_S.setArg(1, alphasumbuff);
		ker_S.setArg(9, Vsumbuff);
		ker_S.setArg(11, Intsumbuff);
		n0[1] = n/n_block[0];
		//n0[2] = n/n_block[0];
		//n0[3] = n;
		queue.enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);		
	
		while(n0[2] > 1)
		{			
			cl::NDRange gsizeRed(0.5*n*n0[2]);		
			queue.enqueueNDRangeKernel(ker_Sp, offset, gsizeRed, local_size);
			n0[2] = 0.5*n0[2];			
		}
		
		queue.enqueueNDRangeKernel(ker_S, offset, gsize1, local_size);


		//counter++;
		
		queue.enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		queue.enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size); 	// Rotational Kick

		queue.enqueueNDRangeKernel(ker_t, offset, gsize1m, unitsize);		// Make positions relative to particle 1
		queue.enqueueNDRangeKernel(ker_t0, offset, unitsize, unitsize);	
		queue.finish();

	}
	
	//clock_t t_elap = clock()-t0;
	time_t t_elap = difftime(time(NULL),t0);	
	float est_time = (1.0/(double) (t_test))*float(t_elap)*(float(max_time)/fabs(stuff[0]));
	int est_h = floor(est_time/3600);
	int est_m = floor((est_time/60)-60*est_h);
	int est_s = floor(est_time-60*est_m-3600*est_h);
	std::cout << "First " << t_test << " steps runtime: " << float(t_elap) << "s; Estimated run time: " << est_h << "h" << est_m << "m" << est_s << "s\n";

	while (t_now < max_time && t_now >= 0)
	{
		queue.enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick

		queue.enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size);	// Rotational Kick

		queue.enqueueNDRangeKernel(ker_T, offset, gsize1, local_size); 		// Evaluate Kinetic Energy
		queue.finish();		
		

		if (( t_now == 0 || fabs(t_now - t_last) >= (1.0 / 64.0)*warp) && framecount < n_frames)
		{
			framecount++;
			if (floor(100 * framecount / n_frames) > 4+floor(prog)){prog = floor(100 * framecount / n_frames); std::cout << prog << "%\n";}		
//			if (floor(100 * framecount / n_frames) > 1+floor(prog))
//			{
//				prog = floor(100 * framecount / n_frames); 
//				std::cout << "*";
//				std::cout.flush();
//			}
			
			queue.enqueueReadBuffer(rbuff, CL_TRUE, ::size_t (0), vecsize, r);
			tempstring = arraytostring(r,n);
			r_tracker << tempstring;

			queue.enqueueReadBuffer(Vbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			V_tracker << tempstring;

			queue.enqueueReadBuffer(Intbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
//			tempstring = arraytostring(stuff, 11);
			E_tracker << tempstring;

			queue.enqueueReadBuffer(Tvbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tv_tracker << tempstring;
		
			queue.enqueueReadBuffer(Twbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tw_tracker << tempstring;

			tempstring = arraytostring(stuff,11);
			stuff_tracker << tempstring;
			
//			queue.enqueueReadBuffer(accelbuff, CL_TRUE, ::size_t (0), vecsize, w);
//			tempstring = arraytostring(w, n);
//			w_tracker << tempstring;


//			r_mean[0][0] = 0.0;
//			r_mean[0][1] = 0.0;
//			r_mean[0][2] = 0.0;
//			for (int i = 0; i < n - 1; i++)
//			{
//				r_mean[0][0] += r[i][0]/(n-1);
//				r_mean[0][1] += r[i][1]/(n-1);
//				r_mean[0][2] += r[i][2]/(n-1);				
//			}
//			queue.enqueueWriteBuffer(r_mean_buff, CL_TRUE, ::size_t (0), ::size_t(32*n), r_mean);	
//			queue.enqueueNDRangeKernel(ker_t_mean, offset, gsize1, unitsize);		// Make positions relative to mean	

			

			t_last = t_now;


		

		}
		queue.enqueueReadBuffer(tbuff, CL_TRUE, ::size_t(0), sizeof(stuff), stuff);
		t_now += stuff[0];
		queue.enqueueNDRangeKernel(ker_scale,offset,local_size,local_size); 	// Set new time step
		
		queue.enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
		queue.enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things
		
	
		queue.enqueueNDRangeKernel(ker_r, offset, gsize1, local_size); 		// Drift

		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
		{
			int a = l3[i];
			int b = l4[i];
			int waitcount = 0;
			//std::cout << offsets_0[i] << "\n";
			queue.flush();
			ker_F.setArg(11, vincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(12, wincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(15, Vincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(16, Intincbuff[a*(n/n_block[0])+b]);
			ker_F.setArg(13, vincbuff[b*(n/n_block[0])+a]);
			ker_F.setArg(14, wincbuff[b*(n/n_block[0])+a]);
			ker_F.setArg(17, Vincbuff[b*(n/n_block[0])+a]);
			ker_F.setArg(18, Intincbuff[b*(n/n_block[0])+a]);			
			ker_F.setArg(21, offset_buff_0[i]);
							
			if (a == b) {queue.enqueueNDRangeKernel(ker_F, offset, gsize2[1], local_size); } 		// Compute force
			else { queue.enqueueNDRangeKernel(ker_F, offset, gsize2[2], local_size); } 
			
		}
		n0[3] = n_block[0];
		n0[2] = n_block[0];
		n0[1] = n_block[0];

		queue.enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);
		
		for (int a = 0; a < (n/n_block[0]); a++)
		{
			for (int b = 0; b < (n/n_block[0]); b++)
			{		
				//int a = l3[i];
				//int b = l4[i];								
				
				n0[2] = n0[1];
				queue.flush();						
				ker_Sp.setArg(0, vincbuff[a*(n/n_block[0])+b]);
				ker_Sp.setArg(1, wincbuff[a*(n/n_block[0])+b]);
				ker_Sp.setArg(3, Vincbuff[a*(n/n_block[0])+b]);
				ker_Sp.setArg(4, Intincbuff[a*(n/n_block[0])+b]);
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue.enqueueNDRangeKernel(ker_Sp, offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}				
							
			}				
		}
		//std::cout << "c";
		
		n0[1] = n;
		n0[2] = n/n_block[0];
		n0[3] = n;
		queue.enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
		{	

			int a = l3[i];
			int b = l4[i];				
			cl::NDRange gsizeRed(n_block[0]);
	
			if (a != b)
			{
			
				queue.flush();
				ker_S_interm.setArg(0, vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(1, wincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(6, Vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(8, Intincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(9, offset_buff_1[a*(n/n_block[0])+b]);			
						
				queue.enqueueNDRangeKernel(ker_S_interm, offset, gsizeRed, local_size);
						
				queue.flush();
				ker_S_interm.setArg(0, vincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(1, wincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(6, Vincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(8, Intincbuff[b*(n/n_block[0])+a]);
				ker_S_interm.setArg(9, offset_buff_1[b*(n/n_block[0])+a]);			
					
				queue.enqueueNDRangeKernel(ker_S_interm, offset, gsizeRed, local_size);
							
			}
			else
			{
				queue.flush();
				ker_S_interm.setArg(0, vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(1, wincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(6, Vincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(8, Intincbuff[a*(n/n_block[0])+b]);
				ker_S_interm.setArg(9, offset_buff_1[a*(n/n_block[0])+b]);			
						
				queue.enqueueNDRangeKernel(ker_S_interm, offset, gsizeRed, local_size);
				
			}
		}

		//std::cout << "d";

		ker_Sp.setArg(0, accelsumbuff);
		ker_Sp.setArg(1, alphasumbuff);
		ker_Sp.setArg(3, Vsumbuff);
		ker_Sp.setArg(4, Intsumbuff);
		ker_S.setArg(0, accelsumbuff);
		ker_S.setArg(1, alphasumbuff);
		ker_S.setArg(9, Vsumbuff);
		ker_S.setArg(11, Intsumbuff);
		n0[1] = n/n_block[0];
		//n0[2] = n/n_block[0];
		//n0[3] = n;
		queue.enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);		
	
		while(n0[2] > 1)
		{			
			cl::NDRange gsizeRed(0.5*n*n0[2]);		
			queue.enqueueNDRangeKernel(ker_Sp, offset, gsizeRed, local_size);
			n0[2] = 0.5*n0[2];			
		}

		queue.enqueueNDRangeKernel(ker_S, offset, gsize1, local_size);


		//counter++;
		
		queue.enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		queue.enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size); 	// Rotational Kick

		queue.enqueueNDRangeKernel(ker_t, offset, gsize1m, unitsize);		// Make positions relative to particle 1
		queue.enqueueNDRangeKernel(ker_t0, offset, unitsize, unitsize);	
		queue.finish();
	}
  	queue.finish();

	queue.enqueueReadBuffer(rbuff, CL_TRUE, ::size_t (0), vecsize, r);
	tempstring = arraytostring(r,n);
	r_final << tempstring;

	queue.enqueueReadBuffer(vbuff, CL_TRUE, ::size_t (0), vecsize, v);
	tempstring = arraytostring(v,n);
	v_final << tempstring;

	queue.enqueueReadBuffer(wbuff, CL_TRUE, ::size_t (0), vecsize, w);
	tempstring = arraytostring(w,n);
	w_final << tempstring;

	std::cout << "\nSimulation complete!\n\n";
	t_elap = difftime(time(NULL),t0);
	std::cout << "Runtime was: " << float(t_elap) << "s\n";
	delete [] r;
	delete [] v;
	delete [] w;	
	delete [] q;
	delete [] m;
	delete [] rad;	
	delete [] E_temp;
	delete [] I;
	delete [] l1;
	delete [] l2;
	delete [] l4;
	delete [] l3;
	delete [] surf_temp;
	delete [] weights;
	delete [] groups;
	
	//long hold = 0;
	//while (true)
	//{
		//hold++;
		//hold--;
	//}
	return 0;
}
