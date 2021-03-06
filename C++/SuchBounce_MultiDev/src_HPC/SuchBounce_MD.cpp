#include "..\Settings.h"

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

std::string arraytostring(double a[n][4], int n)				// converts n-by-4 array to string format [a,b,c,d;e,f,g,h...]
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

cl::CommandQueue queue_init(cl::Context ctx, cl::Device Dev)
{
	cl::CommandQueue tempqueue(ctx, Dev);
	return tempqueue; 
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

cl::Buffer buffer_init(cl::Context ctx, ::size_t size)
{
	cl::Buffer new_buff(ctx, CL_MEM_READ_WRITE, size);
	return new_buff;
}

cl::Buffer buffer_init(cl::Context ctx, ::size_t size, int a)
{
	cl::Buffer new_buff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, &a);
	return new_buff;
}

cl::Buffer buffer_init(cl::Context ctx, ::size_t size, int* a)
{
	cl::Buffer new_buff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, a);
	return new_buff;
}

cl::Buffer buffer_init(cl::Context ctx, ::size_t size, double a[11])
{
	cl::Buffer new_buff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, a);
	return new_buff;
}

cl::Buffer buffer_init(cl::Context ctx, ::size_t size, double a[n][4])
{
	cl::Buffer new_buff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, a);
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

	auto incscaltmp = new double[n_block[0]*n_block[0]];
	auto incvectmp = new double[n_block[0]*n_block[0]*4];
	auto incscal0 = new double[n_block[0]*n_block[0]];
	auto incvec0 = new double[n_block[0]*n_block[0]*4];
	auto vincset = new double[n*n/n_block[0]*4];
	auto wincset = new double[n*n/n_block[0]*4];
	auto Vincset = new double[n*n/n_block[0]];
	auto Intincset = new double[n*n/n_block[0]];

	for (int i = 0; i < n_block[0]*n_block[0]; i++)
	{
		for (int j = 0; j < 4; j++) { incvec0[4*i+j] = 0.0;}
		incscal0[i] = 0.0;
	}
	
	int n0[6] = { n_el, n, n , n, n_block[0], n};

	for (int x=0; x<n; x++)
	{
		I[x] = (2.0*m[x]*(pow(rad[x],2.0))/5.0);		
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

	// OpenCL context setup
	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> platformDevices, allDevices, conDev;
	std::string device_name;
	cl::Device DevChoice;
	int nDev;
	cl::Platform::get(&platforms);
	platforms[platform_choice].getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);	
		
	std::cout << "Please Choose Device:\n\n";
	for (int i = 0; i < platformDevices.size(); i++)
	{
		device_name = platformDevices[i].getInfo<CL_DEVICE_NAME>();
		std::cout << "Device " << i << ": "
		<< device_name
		<< std::endl;
	}	
	std::vector<cl::Device> ctxDevices = { platformDevices[Devs[0]] };
	for (int i = 1; i < nDevs; i++)
	{
		ctxDevices += { platformDevices[Devs[i]] };
	}
	cl::Context ctx(ctxDevices);
	std::vector<cl::CommandQueue> queue = { queue_init(ctx, ctxDevices[0]) };
	for (int i = 1; i < nDevs; i++)
	{
		queue += { queue_init(ctx, ctxDevices[i]) };
	}

	//std::cout << "1\n"; std::cout.flush();

	root = "../Outputs";	
	path = root + "/";
	std::ofstream OpenCL_log(path + "OpenCL_log.txt", std::ios::out);
	
		
	// Build Kernels	
	std::vector<cl::Kernel> ker_F = { kernel_init("F_Hybrid_mat.cl", "Fimp", ctx, ctxDevices, OpenCL_log) };
	std::vector<cl::Kernel> ker_Sp = { kernel_init("reduce_pairwise.cl", "red_pair", ctx, ctxDevices, OpenCL_log) };
	std::vector<cl::Kernel> ker_S_interm = { kernel_init("reduce_mat_interm.cl", "red_copy", ctx, ctxDevices, OpenCL_log) };

				
	cl::Kernel ker_Sp0 = kernel_init("reduce_pairwise.cl", "red_pair", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_T = kernel_init("kinetic.cl", "Tstep", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_r = kernel_init("position.cl", "rstep", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_S = kernel_init("reduce_mat.cl", "red", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_t = kernel_init("translate.cl", "rmove", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_t0 = kernel_init("translate_0.cl", "rmove0", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_t_mean = kernel_init("translate_mean.cl", "rmove", ctx, ctxDevices, OpenCL_log);
    	cl::Kernel ker_v_0 = kernel_init("velocity_mat.cl", "vstep", ctx, ctxDevices, OpenCL_log);
   	cl::Kernel ker_v_1 = kernel_init("velocity_mat.cl", "vstep", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_0_0 = kernel_init("zero.cl", "zeroer", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_0_1 = kernel_init("zero.cl", "zeroer", ctx, ctxDevices, OpenCL_log);
	cl::Kernel ker_0_2 = kernel_init("zero_vec.cl", "zeroer", ctx, ctxDevices, OpenCL_log);
	
	//cl::Kernel ker_scale = kernel_init("time_scaler.cl", "Scale", ctx, ctxDevices, OpenCL_log);		
	

	//std::cout << "2\n"; std::cout.flush();

	// Assign Buffers
	double zerotemp_4[4] = { 0.0, 0.0, 0.0, 0.0 }; 
	::size_t vecsize = ::size_t(4*n*8);

	std::vector<cl::Buffer> nbuff = { buffer_init(ctx, sizeof(n0), n0) };;	
	std::vector<cl::Buffer> l2buff = { buffer_init(ctx, ::size_t(4*n_el), l2) };
	std::vector<cl::Buffer> l1buff = { buffer_init(ctx, ::size_t(4*n_el), l1) };
	
	std::vector<cl::Buffer> cbuff = { buffer_init(ctx, ::size_t(8*n), q) };
	std::vector<cl::Buffer>	mbuff = { buffer_init(ctx, ::size_t(8*n), m) };
	std::vector<cl::Buffer>	radbuff = { buffer_init(ctx, ::size_t(8*n), rad) };
	std::vector<cl::Buffer>	Ibuff = { buffer_init(ctx, ::size_t(8*n), I) };
	std::vector<cl::Buffer>	tbuff = { buffer_init(ctx, sizeof(stuff), stuff) };

	std::vector<cl::Buffer> rbuff = { buffer_init(ctx, vecsize, r) };
	cl::Buffer r_obs_buff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, r);
	std::vector<cl::Buffer> vbuff = { buffer_init(ctx, vecsize, v) };
	std::vector<cl::Buffer> wbuff = { buffer_init(ctx, vecsize, w) };	

	std::vector<cl::Buffer> vincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0]*4)) };
	std::vector<cl::Buffer> wincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0]*4)) };
	std::vector<cl::Buffer> offset_buff_0 = { buffer_init(ctx,::size_t (4), 0) };
	std::vector<cl::Buffer> offset_buff_1 = { buffer_init(ctx,::size_t (4), 0) };
	std::vector<cl::Buffer> Vincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0])) };
	std::vector<cl::Buffer> Intincbuff = { buffer_init(ctx,::size_t (8*n_block[0]*n_block[0])) };
	std::vector<cl::Buffer> Fbuff = { buffer_init(ctx, ::size_t (8*n_el)) };

	for (int i = 1; i < nDevs; i++)
	{
		rbuff += { buffer_init(ctx, vecsize, r) };		
		vbuff += { buffer_init(ctx, vecsize, v) };		
		wbuff += { buffer_init(ctx, vecsize, w) };

		nbuff += { buffer_init(ctx, sizeof(n0), n0) };
		l2buff += { buffer_init(ctx, ::size_t(4*n_el), l2) };
		l1buff += { buffer_init(ctx, ::size_t(4*n_el), l1) };
		cbuff += { buffer_init(ctx, ::size_t(8*n), q) };
		mbuff += { buffer_init(ctx, ::size_t(8*n), m) };
		radbuff += { buffer_init(ctx, ::size_t(8*n), rad) };
		Ibuff += { buffer_init(ctx, ::size_t(8*n), I) };

		tbuff += { buffer_init(ctx, sizeof(stuff), stuff) };
		Fbuff += { buffer_init(ctx, ::size_t (8*n_el)) };		
		
		ker_Sp += { kernel_init("reduce_pairwise_MD.cl", "red_pair", ctx, ctxDevices, OpenCL_log) };
		ker_S_interm += { kernel_init("reduce_MD_interm.cl", "red_copy", ctx, ctxDevices, OpenCL_log) };
		ker_F += { kernel_init("F_Hybrid_mat.cl", "Fimp", ctx, ctxDevices, OpenCL_log) };
	}



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
	}
	
	for (int i = 1; i<pow(n,2)/pow(n_block[0],2); i++)
	{
		offset_buff_1 += { buffer_init(ctx, ::size_t (4), offsets_1[i]) };
	}
	
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
	for(int i = 0; i < nDevs; i++)	
	{
		ker_F[i].setArg(0, cbuff[i]);
		ker_F[i].setArg(1, mbuff[i]);
		ker_F[i].setArg(2, Ibuff[i]);
		ker_F[i].setArg(3, l1buff[i]);
		ker_F[i].setArg(4, l2buff[i]);
		ker_F[i].setArg(5, radbuff[i]);
		ker_F[i].setArg(6, tbuff[i]);
		ker_F[i].setArg(7, rbuff[i]);
		ker_F[i].setArg(8, vbuff[i]);
		ker_F[i].setArg(9, wbuff[i]);
		ker_F[i].setArg(10, wbuff[i]);		
		ker_F[i].setArg(19, Fbuff[i]);
		ker_F[i].setArg(20, nbuff[i]);		

		ker_Sp[i].setArg(2, nbuff[i]);

		ker_S_interm[i].setArg(2, accelsumbuff);
		ker_S_interm[i].setArg(3, alphasumbuff);
		ker_S_interm[i].setArg(4, nbuff[i]);
		ker_S_interm[i].setArg(5, Vsumbuff);
		ker_S_interm[i].setArg(7, Intsumbuff);	
	}
	
	ker_Sp0.setArg(0, accelsumbuff);
	ker_Sp0.setArg(1, alphasumbuff);
	ker_Sp0.setArg(2, nbuff[0]);
	ker_Sp0.setArg(3, Vsumbuff);
	ker_Sp0.setArg(4, Intsumbuff);	

	ker_S.setArg(0, accelsumbuff);
	ker_S.setArg(1, alphasumbuff);		
	ker_S.setArg(2, accelbuff);
	ker_S.setArg(3, alphabuff);
	ker_S.setArg(4, mbuff[0]);
	ker_S.setArg(5, Ibuff[0]);
	ker_S.setArg(6, radbuff[0]);
	ker_S.setArg(7, nbuff[0]);
	ker_S.setArg(8, Vbuff);
	ker_S.setArg(9, Vsumbuff);
	ker_S.setArg(10, Intbuff);
	ker_S.setArg(11, Intsumbuff);	

	ker_0_0.setArg(0, accelbuff);
	ker_0_0.setArg(1, Vbuff);
	ker_0_1.setArg(0, alphabuff);
	ker_0_1.setArg(1, Intbuff);

	ker_0_2.setArg(0, wbuff[0]);	

	ker_v_0.setArg(0, vbuff[0]);
	ker_v_0.setArg(1, accelbuff);	
	ker_v_1.setArg(0, wbuff[0]);
	ker_v_1.setArg(1, alphabuff);

	ker_r.setArg(0, tbuff[0]);
	ker_r.setArg(1, rbuff[0]);
	ker_r.setArg(2, vbuff[0]);	
	
	ker_t.setArg(0,rbuff[0]);

	ker_t0.setArg(0,rbuff[0]);

	ker_T.setArg(0, vbuff[0]);
	ker_T.setArg(1, wbuff[0]);
	ker_T.setArg(2, Tvbuff);
	ker_T.setArg(3, Twbuff);
	ker_T.setArg(4, mbuff[0]);
	ker_T.setArg(5, Ibuff[0]);
	
	cl::NDRange offset(0);
	cl::NDRange gsize1(n);
	cl::NDRange gsize1m(n-1);
	cl::NDRange unitsize(1);
	std::vector<cl::NDRange> gsize2 = { ::size_t(n_el), ::size_t(n_block[1]*n_block[2]), ::size_t(n_block[0]*n_block[0]) };
	cl::NDRange local_size(workgroup_size);
	std::cout << "Kernels & Buffers set...";

	// Initialise simulation
	double t_now = 0;	
	double t_last = 0;
	if (stuff[0] < 0) { t_now = max_time; t_last = max_time;}
	short int prog = 0;

	double r_mean[n][4];
	count[0] = 0;
	std::string tempstring;	
	
	queue[0].enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
	queue[0].enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things	
	
	std::cout << "..simulation started.\n";

	std::cout << "\n";

	time_t t0 = time(NULL);	
	
	for(int init_x=0; init_x<t_test; init_x ++)
	{			
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		
		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size);	// Rotational Kick
		
		queue[0].enqueueNDRangeKernel(ker_T, offset, gsize1, local_size); 		// Evaluate Kinetic Energy
		queue[0].finish();
		
		//! UPDATE variables between devices here
//		queue[0].enqueueReadBuffer(rbuff[0], CL_TRUE, ::size_t (0), vecsize, r);
		queue[0].enqueueReadBuffer(vbuff[0], CL_TRUE, ::size_t (0), vecsize, v);
		queue[0].enqueueReadBuffer(wbuff[0], CL_TRUE, ::size_t (0), vecsize, w);
		
		for (int i = 1; i < nDevs; i++)
		{
//			queue[i].enqueueWriteBuffer(rbuff[i], CL_TRUE, ::size_t (0), vecsize, r);
			queue[i].enqueueWriteBuffer(vbuff[i], CL_TRUE, ::size_t (0), vecsize, v);
			queue[i].enqueueWriteBuffer(wbuff[i], CL_TRUE, ::size_t (0), vecsize, w);
		}

		if (( t_now == 0 || fabs(t_now - t_last) >= (1.0 / 64.0)*warp) && framecount < n_frames)		//  Writes out current values of Energies and position to file
		{
			framecount++;
			if (floor(100 * framecount / n_frames) > 4+floor(prog)){prog = floor(100 * framecount / n_frames); std::cout << prog << "%\n";}				
			
			tempstring = arraytostring(r,n);
			r_tracker << tempstring;			

			queue[0].enqueueReadBuffer(Vbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			V_tracker << tempstring;

			queue[0].enqueueReadBuffer(Intbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			E_tracker << tempstring;

			queue[0].enqueueReadBuffer(Tvbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tv_tracker << tempstring;
		
			queue[0].enqueueReadBuffer(Twbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tw_tracker << tempstring;

			tempstring = arraytostring(stuff,11);
			stuff_tracker << tempstring;			

			t_last = t_now;		
		}
		
		queue[0].enqueueReadBuffer(tbuff[0], CL_TRUE, ::size_t(0), sizeof(stuff), stuff);
		t_now += stuff[0];		
		
		queue[0].enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
		queue[0].enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things		
	
		queue[0].enqueueNDRangeKernel(ker_r, offset, gsize1, local_size); 		// Drift
	
		//! UPDATE variables between devices here
		queue[0].enqueueReadBuffer(rbuff[0], CL_TRUE, ::size_t (0), vecsize, r);
//		queue[0].enqueueReadBuffer(vbuff[0], CL_TRUE, ::size_t (0), vecsize, v);
//		queue[0].enqueueReadBuffer(wbuff[0], CL_TRUE, ::size_t (0), vecsize, w);
		
		for (int i = 1; i < nDevs; i++)
		{
			queue[i].enqueueWriteBuffer(rbuff[i], CL_TRUE, ::size_t (0), vecsize, r);
//			queue[i].enqueueWriteBuffer(vbuff[i], CL_TRUE, ::size_t (0), vecsize, v);
//			queue[i].enqueueWriteBuffer(wbuff[i], CL_TRUE, ::size_t (0), vecsize, w);
		}
	

		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++) 				// Loop over all interaction blocks
		{
			int a = l3[i];
			int b = l4[i];

			int ind = i - nDevs*floor(i/nDevs);			
			//queue[ind].flush();			
			queue[ind].finish();	
			ker_F[ind].setArg(11, vincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(12, wincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(15, Vincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(16, Intincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(13, vincbuff[b*(n/n_block[0])+a]);
			ker_F[ind].setArg(14, wincbuff[b*(n/n_block[0])+a]);
			ker_F[ind].setArg(17, Vincbuff[b*(n/n_block[0])+a]);
			ker_F[ind].setArg(18, Intincbuff[b*(n/n_block[0])+a]);			
			ker_F[ind].setArg(21, offset_buff_0[i]);		
							
			if (a == b) {queue[ind].enqueueNDRangeKernel(ker_F[ind], offset, gsize2[1], local_size); } 		// Compute force 
			else { queue[ind].enqueueNDRangeKernel(ker_F[ind], offset, gsize2[2], local_size); } 				// (for off-diagonal blocks, n_block^2 unique elements, 0.5*n_block*(n_block)-1 for diagonal)			
		}
		
		n0[3] = n_block[0];				// 
		n0[2] = n_block[0];
		n0[1] = n_block[0];

		for (int i = 0; i < nDevs; i++)	
		{
			queue[ind].finish();
			queue[i].enqueueWriteBuffer(nbuff[i], CL_TRUE, ::size_t (0), sizeof(n0), n0);
		}
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++) 														// Perform pairwise sum of all interaction quantities for each interaction block
		{																																		
			int a = l3[i];
			int b = l4[i];

			int ind = i - nDevs*floor(i/nDevs);			
			//queue[ind].flush();
			queue[ind].finish();
			n0[2] = n0[1];
			
			ker_Sp[ind].setArg(0, vincbuff[a*(n/n_block[0])+b]);
			ker_Sp[ind].setArg(1, wincbuff[a*(n/n_block[0])+b]);
			ker_Sp[ind].setArg(3, Vincbuff[a*(n/n_block[0])+b]);
			ker_Sp[ind].setArg(4, Intincbuff[a*(n/n_block[0])+b]);
			while (n0[2] > 1)
			{			
				cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
				queue[ind].enqueueNDRangeKernel(ker_Sp[ind], offset, gsizeRed, local_size);
				n0[2] = 0.5*n0[2];
			}

			if (a != b)																													
			{
				//queue[ind].flush();
				queue[ind].finish();
				n0[2] = n0[1];
			
				ker_Sp[ind].setArg(0, vincbuff[b*(n/n_block[0])+a]);
				ker_Sp[ind].setArg(1, wincbuff[b*(n/n_block[0])+a]);
				ker_Sp[ind].setArg(3, Vincbuff[b*(n/n_block[0])+a]);
				ker_Sp[ind].setArg(4, Intincbuff[b*(n/n_block[0])+a]);
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue[ind].enqueueNDRangeKernel(ker_Sp[ind], offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}						
			}					
		}				
				
		n0[1] = n;
		n0[2] = n/n_block[0];
		n0[3] = n;

		for (int i = 0; i < nDevs; i++)	
		{
			queue[ind].finish();
			queue[i].enqueueWriteBuffer(nbuff[i], CL_TRUE, ::size_t (0), sizeof(n0), n0);
		}
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++) 																						// Copy & collate blocks
		{																																										// |
			int a = l3[i];																																					// |
			int b = l4[i];																																					// |
			int ind = i - nDevs*floor(i/nDevs);																														// |
			cl::NDRange gsizeRed(n_block[0]);																													// |
																																												// |
			//queue[ind].flush();																																		// |
			queue[ind].finish();																																			// |
			ker_S_interm[ind].setArg(0, vincbuff[a*(n/n_block[0])+b]);																					// |
			ker_S_interm[ind].setArg(1, wincbuff[a*(n/n_block[0])+b]);																					// |
			ker_S_interm[ind].setArg(6, Vincbuff[a*(n/n_block[0])+b]);																					// |
			ker_S_interm[ind].setArg(8, Intincbuff[a*(n/n_block[0])+b]);																				// |
			ker_S_interm[ind].setArg(9, offset_buff_1[a*(n/n_block[0])+b]);																			// |
																																												// |
			queue[ind].enqueueNDRangeKernel(ker_S_interm[ind], offset, gsizeRed, local_size);											// |
			if (a != b)																																						// |
			{																																									// |
				//queue[ind].flush();																																	// |
				queue[ind].finish();																																		// |
				ker_S_interm[ind].setArg(0, vincbuff[b*(n/n_block[0])+a]);																				// |
				ker_S_interm[ind].setArg(1, wincbuff[b*(n/n_block[0])+a]);																				// |
				ker_S_interm[ind].setArg(6, Vincbuff[b*(n/n_block[0])+a]);																				// |
				ker_S_interm[ind].setArg(8, Intincbuff[b*(n/n_block[0])+a]);																			// |
				ker_S_interm[ind].setArg(9, offset_buff_1[b*(n/n_block[0])+a]);																		// |
																																												// |
				queue[ind].enqueueNDRangeKernel(ker_S_interm[ind], offset, gsizeRed, local_size);										// |
			}																																									// |
		}																																										//---
			
		n0[1] = n/n_block[0];
		
		for (int i = 0; i < nDevs; i++)	
		{
			queue[ind].finish();
			queue[i].enqueueWriteBuffer(nbuff[i], CL_TRUE, ::size_t (0), sizeof(n0), n0);
		}
		
		while(n0[2] > 1) 																													// | Perform pairwise sum of collated block
		{																																		// |
			cl::NDRange gsizeRed(0.5*n*n0[2]);																				// |
			queue[0].enqueueNDRangeKernel(ker_Sp0, offset, gsizeRed, local_size);							// |
			n0[2] = 0.5*n0[2];																											// |
		}																																		//---
		
		queue[0].enqueueNDRangeKernel(ker_S, offset, gsize1, local_size);		// Copy summed values to relevant system state buffers
		
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick	
		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size); 	// Rotational Kick			

		queue[0].enqueueNDRangeKernel(ker_t, offset, gsize1m, unitsize);		// Make positions relative to particle 1
		queue[0].enqueueNDRangeKernel(ker_t0, offset, unitsize, unitsize);	
		queue[0].finish();

	}
	
	time_t t_elap = difftime(time(NULL),t0);	
	float est_time = (1.0/(double) (t_test))*float(t_elap)*(float(max_time)/fabs(stuff[0]));
	int est_h = floor(est_time/3600);
	int est_m = floor((est_time/60)-60*est_h);
	int est_s = floor(est_time-60*est_m-3600*est_h);
	std::cout << "First " << t_test << " steps runtime: " << float(t_elap) << "s; Estimated run time: " << est_h << "h" << est_m << "m" << est_s << "s\n";

	while (t_now < max_time && t_now >= 0)
	{
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		
		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size);	// Rotational Kick
		
		queue[0].enqueueNDRangeKernel(ker_T, offset, gsize1, local_size); 		// Evaluate Kinetic Energy
		queue[0].finish();
		
		//! UPDATE variables between devices here
//		queue[0].enqueueReadBuffer(rbuff[0], CL_TRUE, ::size_t (0), vecsize, r);
		queue[0].enqueueReadBuffer(vbuff[0], CL_TRUE, ::size_t (0), vecsize, v);
		queue[0].enqueueReadBuffer(wbuff[0], CL_TRUE, ::size_t (0), vecsize, w);
		
		for (int i = 1; i < nDevs; i++)
		{
//			queue[i].enqueueWriteBuffer(rbuff[i], CL_TRUE, ::size_t (0), vecsize, r);
			queue[i].enqueueWriteBuffer(vbuff[i], CL_TRUE, ::size_t (0), vecsize, v);
			queue[i].enqueueWriteBuffer(wbuff[i], CL_TRUE, ::size_t (0), vecsize, w);
		}

		if (( t_now == 0 || fabs(t_now - t_last) >= (1.0 / 64.0)*warp) && framecount < n_frames)
		{
			framecount++;
			if (floor(100 * framecount / n_frames) > 4+floor(prog)){prog = floor(100 * framecount / n_frames); std::cout << prog << "%\n";}				
			
			tempstring = arraytostring(r,n);
			r_tracker << tempstring;
			
			queue[0].enqueueReadBuffer(Vbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			V_tracker << tempstring;

			queue[0].enqueueReadBuffer(Intbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			E_tracker << tempstring;

			queue[0].enqueueReadBuffer(Tvbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tv_tracker << tempstring;
		
			queue[0].enqueueReadBuffer(Twbuff, CL_TRUE, ::size_t (0), ::size_t(8*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tw_tracker << tempstring;

			tempstring = arraytostring(stuff,11);
			stuff_tracker << tempstring;			

			t_last = t_now;		

		}
		
		queue[0].enqueueReadBuffer(tbuff[0], CL_TRUE, ::size_t(0), sizeof(stuff), stuff);
		t_now += stuff[0];		
		
		queue[0].enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
		queue[0].enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things		
	
		queue[0].enqueueNDRangeKernel(ker_r, offset, gsize1, local_size); 		// Drift	

		//! UPDATE variables between devices here
		queue[0].enqueueReadBuffer(rbuff[0], CL_TRUE, ::size_t (0), vecsize, r);
//		queue[0].enqueueReadBuffer(vbuff[0], CL_TRUE, ::size_t (0), vecsize, v);
//		queue[0].enqueueReadBuffer(wbuff[0], CL_TRUE, ::size_t (0), vecsize, w);
		
		for (int i = 1; i < nDevs; i++)
		{
			queue[i].enqueueWriteBuffer(rbuff[i], CL_TRUE, ::size_t (0), vecsize, r);
//			queue[i].enqueueWriteBuffer(vbuff[i], CL_TRUE, ::size_t (0), vecsize, v);
//			queue[i].enqueueWriteBuffer(wbuff[i], CL_TRUE, ::size_t (0), vecsize, w);
		}

		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
		{
			int a = l3[i];
			int b = l4[i];

			int ind = i - nDevs*floor(i/nDevs);			
			//queue[ind].flush();
			queue[ind].finish();
			ker_F[ind].setArg(11, vincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(12, wincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(15, Vincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(16, Intincbuff[a*(n/n_block[0])+b]);
			ker_F[ind].setArg(13, vincbuff[b*(n/n_block[0])+a]);
			ker_F[ind].setArg(14, wincbuff[b*(n/n_block[0])+a]);
			ker_F[ind].setArg(17, Vincbuff[b*(n/n_block[0])+a]);
			ker_F[ind].setArg(18, Intincbuff[b*(n/n_block[0])+a]);			
			ker_F[ind].setArg(21, offset_buff_0[i]);		
							
			if (a == b) {queue[ind].enqueueNDRangeKernel(ker_F[ind], offset, gsize2[1], local_size); } 		// Compute force
			else { queue[ind].enqueueNDRangeKernel(ker_F[ind], offset, gsize2[2], local_size); } 					
		}
		
		n0[3] = n_block[0];
		n0[2] = n_block[0];
		n0[1] = n_block[0];

		for (int i = 0; i < nDevs; i++)	
		{
			queue[ind].finish();
			queue[i].enqueueWriteBuffer(nbuff[i], CL_TRUE, ::size_t (0), sizeof(n0), n0);
		}
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
		{			
			int a = l3[i];
			int b = l4[i];

			int ind = i - nDevs*floor(i/nDevs);			
			//queue[ind].flush();
			queue[ind].finish();
			n0[2] = n0[1];			
			
			ker_Sp[ind].setArg(0, vincbuff[a*(n/n_block[0])+b]);
			ker_Sp[ind].setArg(1, wincbuff[a*(n/n_block[0])+b]);
			ker_Sp[ind].setArg(3, Vincbuff[a*(n/n_block[0])+b]);
			ker_Sp[ind].setArg(4, Intincbuff[a*(n/n_block[0])+b]);
			while (n0[2] > 1)
			{			
				cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
				queue[ind].enqueueNDRangeKernel(ker_Sp[ind], offset, gsizeRed, local_size);
				n0[2] = 0.5*n0[2];
			}
			if (a != b)
			{
				//queue[ind].flush();
				queue[ind].finish();
				n0[2] = n0[1];
			
				ker_Sp[ind].setArg(0, vincbuff[b*(n/n_block[0])+a]);
				ker_Sp[ind].setArg(1, wincbuff[b*(n/n_block[0])+a]);
				ker_Sp[ind].setArg(3, Vincbuff[b*(n/n_block[0])+a]);
				ker_Sp[ind].setArg(4, Intincbuff[b*(n/n_block[0])+a]);
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue[ind].enqueueNDRangeKernel(ker_Sp[ind], offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}						
			}				
		}				
				
		n0[1] = n;
		n0[2] = n/n_block[0];
		n0[3] = n;

			for (int i = 0; i < nDevs; i++)	
		{
			queue[ind].finish();
			queue[i].enqueueWriteBuffer(nbuff[i], CL_TRUE, ::size_t (0), sizeof(n0), n0);
		}	
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++) // Copy & collate blocks
		{	
			int a = l3[i];
			int b = l4[i];	
			int ind = i - nDevs*floor(i/nDevs);
			cl::NDRange gsizeRed(n_block[0]);	
			
			//queue[ind].flush();
			queue[ind].finish();
			ker_S_interm[ind].setArg(0, vincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[ind].setArg(1, wincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[ind].setArg(6, Vincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[ind].setArg(8, Intincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[ind].setArg(9, offset_buff_1[a*(n/n_block[0])+b]);			
					
			queue[ind].enqueueNDRangeKernel(ker_S_interm[ind], offset, gsizeRed, local_size);
			if (a != b)
			{			
				//queue[ind].flush();
				queue[ind].finish();
				ker_S_interm[ind].setArg(0, vincbuff[b*(n/n_block[0])+a]);
				ker_S_interm[ind].setArg(1, wincbuff[b*(n/n_block[0])+a]);
				ker_S_interm[ind].setArg(6, Vincbuff[b*(n/n_block[0])+a]);
				ker_S_interm[ind].setArg(8, Intincbuff[b*(n/n_block[0])+a]);
				ker_S_interm[ind].setArg(9, offset_buff_1[b*(n/n_block[0])+a]);			
					
				queue[ind].enqueueNDRangeKernel(ker_S_interm[ind], offset, gsizeRed, local_size);
			}
		}
	
		n0[1] = n/n_block[0];
		
			for (int i = 0; i < nDevs; i++)	
		{
			queue[ind].finish();
			queue[i].enqueueWriteBuffer(nbuff[i], CL_TRUE, ::size_t (0), sizeof(n0), n0);
		}		
	
		while(n0[2] > 1)
		{			
			cl::NDRange gsizeRed(0.5*n*n0[2]);		
			queue[0].enqueueNDRangeKernel(ker_Sp0, offset, gsizeRed, local_size);
			n0[2] = 0.5*n0[2];			
		}
		
		queue[0].enqueueNDRangeKernel(ker_S, offset, gsize1, local_size);
		
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick	
		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size); 	// Rotational Kick			

		queue[0].enqueueNDRangeKernel(ker_t, offset, gsize1m, unitsize);		// Make positions relative to particle 1
		queue[0].enqueueNDRangeKernel(ker_t0, offset, unitsize, unitsize);	
		queue[0].finish();		

	}
  	queue[0].finish();

	queue[0].enqueueReadBuffer(rbuff[0], CL_TRUE, ::size_t (0), vecsize, r);
	tempstring = arraytostring(r,n);
	r_final << tempstring;
	r_final.flush();

	queue[0].enqueueReadBuffer(vbuff[0], CL_TRUE, ::size_t (0), vecsize, v);
	tempstring = arraytostring(v,n);
	v_final << tempstring;
	v_final.flush();

	queue[0].enqueueReadBuffer(wbuff[0], CL_TRUE, ::size_t (0), vecsize, w);
	tempstring = arraytostring(w,n);
	w_final << tempstring;
	w_final.flush();

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
	
	delete [] incscaltmp;
	delete [] incvectmp;
	delete [] vincset;
	delete [] wincset;
	delete [] Vincset;
	delete [] Intincset;
	
	return 0;
}
