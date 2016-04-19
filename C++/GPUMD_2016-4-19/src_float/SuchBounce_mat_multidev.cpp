#include "../Settings.h"

//Auto-stuff
float stuff[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
float t_step = 0;
const int n_el = (0.5)*n*(n - 1);


// Calculated interval between data samples for output	
const int n_frames = (max_time * 64 / warp);

template <typename T>
std::vector<T> &operator+=(std::vector<T> &A, const std::vector<T> &B)
{
    A.reserve( A.size() + B.size() );                // preallocate memory without erase original data
    A.insert( A.end(), B.begin(), B.end() );         // add B;
    return A;                                        // here A could be named AB
}


int string2array(std::string str, float arr[4])
{
	std::string::size_type end0;
	std::string::size_type end1;
	arr[0] = std::stod(str,&end0);
	std::string c = str.substr(end0+1);
	arr[1] = std::stod(c,&end1);
	arr[2] = std::stod(c.substr(end1+2));
	return 1;
}

std::string arraytostring(float a[n][4], int n)
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

std::string arraytostring(float a[n], int n)
{
	std::string out = "[";
	for (int j = 0; j < n; j++)
	{
			char temp[25];
			sprintf(temp, "%+.8e", a[j]);
			out.append(std::string(temp));
			if (j != (n-1)){ out.append(", "); }
	}
	out.append("]\n");
	return out;
}

cl::Kernel kernel_init(std::string file, std::string ker_func, cl::Context ctx, std::vector<cl::Device> ctxdev)
{
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
	//if (sizeof(a) != size) {std::cout << "You done gone mucked up!\n";}
	return new_buff;
}

cl::CommandQueue queue_init(cl::Context ctx, cl::Device Dev)
{
	cl::CommandQueue tempqueue(ctx, Dev);
	return tempqueue; 
}


int main() 
{
	auto r = new float[n][4];
	auto v = new float[n][4];
	auto w = new float[n][4];	
	auto q = new float[n];
	auto m = new float[n];
	auto rad = new float[n];
	std::string path;
	std::string root = "../Setup";

	path = root + "/";
	std::ifstream r_in(path + "r.vec");
	std::ifstream v_in(path + "v.vec");
	std::ifstream w_in(path + "w.vec");
	std::ifstream rad_in(path + "rad.vec");	
	std::ifstream q_in(path + "q.vec");
	std::ifstream m_in(path + "m.vec");
	
	for (int i = 0; i < n; i++)
	{
		std::string r_str = "";
		std::string v_str = "";
		std::string w_str = "";
		std::string q_str = "";
		std::string m_str = "";
		std::string rad_str = "";				
		float r_temp[4] = {0.0, 0.0, 0.0, 0.0};
		float v_temp[4] = {0.0, 0.0, 0.0, 0.0};
		float w_temp[4] = {0.0, 0.0, 0.0, 0.0};
		float q_temp = 0.0;
		float rad_temp = 0.0;
		float m_temp = 0.0;		

		std::getline(r_in,r_str);
		std::getline(v_in,v_str);
		std::getline(w_in,w_str);
		std::getline(q_in,q_str);
		std::getline(m_in,m_str);
		std::getline(rad_in,rad_str);
		
		string2array(r_str,r_temp);
		string2array(v_str,v_temp);
		string2array(w_str,w_temp);
		
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
		}
	}
	//for (int i = 0; i<n; i++){std::cout << q[i] << "; "; }
	//std::cout <<"\n";

		
	stuff[0] = settings[1];
	stuff[1] = settings[3];
	stuff[2] = settings[4];
	//stuff[3] = settings[5];
	stuff[4] = settings[6];
	stuff[5] = settings[7];
	stuff[6] = 6.67384e-11;
	stuff[7] = pow((4 * 3.141592654*8.85419e-12), -1); // electrostatic force constant
	stuff[8] = settings[8];
	stuff[9] = settings[2];
	stuff[10] = settings[1];

	if (n_frames > max_time/stuff[9])
	{	
		std::cout << "Insufficient frames for specified warp; frame intervals will be wierd\r\n";
		std::cout << n_frames << "; " << (max_time/stuff[9]) << "\n";
	}

	int framecount = 0;
	int tempcount = 0;
	auto E_temp = new float[n];
	auto I = new float[n];
	auto l1 = new int[n_el]; // work element -> interaction pair
	auto l2 = new int[n_el];
	auto l3 = new int[(n/n_block[0])*(n/n_block[0]+1)/2]; // work block -> block pair
	auto l4 = new int[(n/n_block[0])*(n/n_block[0]+1)/2];
	auto l5 = new int[(n/n_block[0])*(n/n_block[0])/2]; // work item -> work block(s)
	auto l6 = new int[(n/n_block[0])*(n/n_block[0])/2];
	auto l1temp = new int[n_block[2]*n_block[1]];
	auto l2temp = new int[n_block[2]*n_block[1]];
	auto l1_blocks = new int[n_block[2]*n_block[1]];
	auto l2_blocks = new int[n_block[2]*n_block[1]];
	
	
	int n0[5] = { n_el, n, n , n, n_block[0]};

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
			l1_blocks[i] = x;
			l2_blocks[i] = y;			
		}
		
	}
	//for (int x=0; x<n/n_block[0]; x++)
	//{
	//	for (int y=0; y<x+1; y++)	
	//	{	
	//		int i = (0.5*x*(x-1)+y);			
	//		l3[i] = x;
	//		l4[i] = y;			
	//	}
	//	
	//}

	
	//for (int p=0; p<(n_block[2]*n_block[1]); p=p+n_block[2])
	//{
	//	int j = 0;
	//	for (int q=0; q<n_block[2]; q++)
	//	{
	//		int i = 0;
	//		int k = 0;			
	//		while(k == 0)			
	//		{	
	//			if (q != 0)
	//			{		
	//				for (int s=0; s<q; s++)
	//				{
	//					if (l1temp[i] == l1_n_block[0][p+s] || l1temp[i] == l2_n_block[0][p+s] || l2temp[i] == l1_n_block[0][p+s]  || l2temp[i] == l2_n_block[0][p+s]){j=1;}   
	//				}
	//			}		
	//			if (j == 0 && l1temp[i] != -1) {l1_n_block[0][p+q] = l1temp[i]; l2_n_block[0][p+q] = l2temp[i]; l1temp[i] = -1; l2temp[i] = -1; k = 1;}
	//			else{i++;}
	//			j = 0;
	//		}
	//	}
	//}

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
		
		//if (count[1] == 0) { offsets_0[count[1]] = 0; }
		//else { offsets_0[count[1]] = offsets_0[count[1] - 1] + n_block[2]*n_block[1];  }
		offsets_0[count[1]+1] = offsets_0[count[1]] + n_block[2]*n_block[1];		
		l3[count[1]] = p; l4[count[1]] = p;
		count[1]++;
		for (int i=0; i<(n_block[2]*n_block[1]); i++)
		{    		
							
			l1[count[0]] = l1_blocks[i] + n_block[0]*p;
    			l2[count[0]] = l2_blocks[i] + n_block[0]*p;
    			count[0]++;
		}
		for (int u = 0; u<(n/n_block[0]); u++)
		{
			offsets_1[(n/n_block[0])*p+u] = n*p+u;
			//std::cout << (n/n_block[0])*(offsets_1[(n/n_block[0])*p+u]/(n/n_block[0])) << ", " << offsets_1[(n/n_block[0])*p+u] - (n/n_block[0])*(offsets_1[(n/n_block[0])*p+u]/(n/n_block[0])) << "; ";
		}
    		
	}
	
	int last_diag = -1;
	int countWI = 0;
	for (int i = 0; i < (n/n_block[0])*(n/n_block[0]+1)/2; i++)
	{	
		if (l3[i] == l4[i])
		{
			if (last_diag == -1) {l5[countWI] = i; last_diag = countWI; countWI++;}
			else { l6[last_diag] = i; last_diag = -1;}
		}
		else { l5[countWI] = i; l6[countWI] = -1; countWI++;} 				
	}

	//for (int i = 0; i < (n/n_block[0])*(n/n_block[0])/2; i++) { std::cout << i << ";" << l5[i] << ";" << l6[i] << "\n";}	

	//int fail = 0;
	//for (int p = 0; p<n_block[2]*n_block[1]; p = p + n_block[2])
	//{
	//	for (int i = 1; i<n_block[2]; i++)
	//	{
	//		for (int j = 0; j<i; j++)
	//		{
	//			if (l1[p+i] == l1[p+j] || l1[p+i] == l2[p+j] || l2[p+i] == l1[p+j] || l2[p+i] == l2[p+j])
	//			{
	//				fail++;
	//			}
	//		}
	//	}


	//}
	//std::cout << "Generated interaction maps...\n";
	delete [] l1temp;
	delete [] l2temp; 
	delete [] l1_blocks;
	delete [] l2_blocks;
	

	// OpenCL Context Sorcery
	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> platformDevices, allDevices, conDev;
	std::string device_name;
	cl::Device DevChoice;
	//int nDevs = 1;
	//int Devs[2] = { 1, 0 };
	cl::Platform::get(&platforms);
	platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);
	
		
	//std::cout << "Devices in Platform:\n\n";
	for (int i = 0; i < platformDevices.size(); i++)
	{
		device_name = platformDevices[i].getInfo<CL_DEVICE_NAME>();
		//std::cout << "Device " << i << ": "
		//<< device_name
		//<< std::endl;
	}
	
	std::vector<cl::Device> ctxDevices = { platformDevices[Devs[0]] };
	
	for (int i = 1; i < nDevs; i++)	
	{							
		ctxDevices += { platformDevices[Devs[i]] };				
	}
	
	 
	//std::vector<cl::Device> ctxDevices = { platformDevices[0] };
	cl::Context ctx(ctxDevices);
	std::vector<cl::CommandQueue> queue = { queue_init(ctx, ctxDevices[0]) };
	for (int i = 1; i < nDevs; i++)
	{
		queue += { queue_init(ctx, ctxDevices[i]) };
	}
		
	
		
	// Build Kernels
	std::vector<cl::Kernel> ker_F = {kernel_init("F_Hybrid_mat.cl", "Fimp", ctx, ctxDevices)};
	//cl::Kernel ker_F = kernel_init("F_Hybrid_mat.cl", "Fimp", ctx, ctxDevices);
	cl::Kernel ker_T = kernel_init("kinetic.cl", "Tstep", ctx, ctxDevices);
	cl::Kernel ker_r = kernel_init("position.cl", "rstep", ctx, ctxDevices);
	cl::Kernel ker_S = kernel_init("reduce_mat.cl", "red", ctx, ctxDevices);
	std::vector<cl::Kernel> ker_Sp = {kernel_init("reduce_pairwise_MD.cl", "red_pair", ctx, ctxDevices)};
	std::vector<cl::Kernel> ker_S_interm = {kernel_init("reduce_MD_interm.cl", "red_copy", ctx, ctxDevices)};
	cl::Kernel ker_Sp0 = kernel_init("reduce_pairwise_MD.cl", "red_pair", ctx, ctxDevices);	
	cl::Kernel ker_t = kernel_init("translate.cl", "rmove", ctx, ctxDevices);
	cl::Kernel ker_t0 = kernel_init("translate_0.cl", "rmove0", ctx, ctxDevices);
	cl::Kernel ker_v_0 = kernel_init("velocity.cl", "vstep", ctx, ctxDevices);
	cl::Kernel ker_v_1 = kernel_init("velocity.cl", "vstep", ctx, ctxDevices);
	cl::Kernel ker_0_0 = kernel_init("zero.cl", "zeroer", ctx, ctxDevices);
	cl::Kernel ker_0_1 = kernel_init("zero.cl", "zeroer", ctx, ctxDevices);
	cl::Kernel ker_scale = kernel_init("time_scaler.cl", "Scale", ctx, ctxDevices);
	
	
	// Assign Buffers
	float zerotemp_4[4] = { 0.0, 0.0, 0.0, 0.0 }; 
	::size_t vecsize = ::size_t(4*n*4);

	cl::Buffer nbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(n0), n0);
	cl::Buffer l2buff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n_el), l2);
	cl::Buffer l1buff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n_el), l1);
	cl::Buffer cbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n), q);
	cl::Buffer mbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n), m);
	cl::Buffer radbuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n), rad);
	cl::Buffer Ibuff(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ::size_t(4*n), I);
	cl::Buffer tbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(stuff), stuff);
	cl::Buffer Ftmp(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ::size_t (16), zerotemp_4);

	cl::Buffer rbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, r);
	cl::Buffer vbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, v);
	cl::Buffer wbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vecsize, w);

	std::vector<cl::Buffer> vincbuff = { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0]*4)) };
	std::vector<cl::Buffer> wincbuff = { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0]*4)) };
	std::vector<cl::Buffer> offset_buff_0 = { buffer_init(ctx,::size_t (4), 0) };
	std::vector<cl::Buffer> offset_buff_1 = { buffer_init(ctx,::size_t (4), 0) };
	std::vector<cl::Buffer> Vincbuff = { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0])) };
	std::vector<cl::Buffer> Intincbuff = { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0])) };
	std::cout << "Attempting to split " << (pow(n,2)/pow(n_block[0],2)) << " ways...\n";
	
	for (int i = 1; i < (n/n_block[0])*(n/n_block[0]+1)/2; i++) 
	{ 
		ker_F += { kernel_init("F_Hybrid_mat.cl", "Fimp", ctx, ctxDevices) }; 		
	}
	for (int i = 1; i < (n/n_block[0])*(n/n_block[0]); i++) 
	{ 
		ker_Sp += { kernel_init("reduce_pairwise_MD.cl", "red_pair", ctx, ctxDevices) };
		ker_S_interm += { kernel_init("reduce_MD_interm.cl", "red_copy", ctx, ctxDevices) };	
	}	
		

	for (int i = 1; i<(pow(n,2)/pow(n_block[0],2)); i++)
	{
		vincbuff += { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0]*4)) };
		wincbuff += { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0]*4)) };
		Vincbuff += { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0])) };
		Intincbuff += { buffer_init(ctx,::size_t (4*n_block[0]*n_block[0])) };		
	}
	
	for (int i = 1; i<0.5*(n/n_block[0])*((n/n_block[0])+1); i++)
	{
		offset_buff_0 += { buffer_init(ctx, ::size_t (4), offsets_0[i]) };				
	}
	
	for (int i = 1; i<pow(n,2)/pow(n_block[0],2); i++)
	{
		offset_buff_1 += { buffer_init(ctx, ::size_t (4), offsets_1[i]) };
		//std::cout << offsets_1[i] << "; ";
		
	}
	
	std::cout << "Split particles into " << vincbuff.size() << " blocks\n";

	
	cl::Buffer accelbuff(ctx, CL_MEM_READ_WRITE, vecsize);
	cl::Buffer alphabuff(ctx, CL_MEM_READ_WRITE, vecsize);

	cl::Buffer accelsumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n*n/n_block[0]*4));
	cl::Buffer alphasumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n*n/n_block[0]*4));
	cl::Buffer Vsumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n*n/n_block[0]));
	cl::Buffer Intsumbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n*n/n_block[0]));

	cl::Buffer Vbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n));
	cl::Buffer Intbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n));
	cl::Buffer Tvbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n));
	cl::Buffer Twbuff(ctx, CL_MEM_READ_WRITE, ::size_t (4*n));	

	
	// Open output streams
	root = "../Outputs";	
	path = root + "/";
	std::ofstream r_tracker(path + "Particle_tracks.dat", std::ios::out);
	std::ofstream Tv_tracker(path + "T_v_tracks.dat", std::ios::out);
	std::ofstream Tw_tracker(path + "T_w_tracks.dat", std::ios::out);
	std::ofstream E_tracker(path + "E_int_tracks.dat", std::ios::out);
	std::ofstream V_tracker(path + "V_tracks.dat", std::ios::out);
	std::ofstream v_tracker(path + "v_tracks.dat", std::ios::out);
	std::ofstream w_tracker(path + "w_tracks.dat", std::ios::out);
	
	

	//Set Kernel Arguments
	for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]+1); i++)
	{
		ker_F[i].setArg(0, cbuff);
		ker_F[i].setArg(1, mbuff);
		ker_F[i].setArg(2, Ibuff);
		ker_F[i].setArg(3, l1buff);
		ker_F[i].setArg(4, l2buff);
		ker_F[i].setArg(5, radbuff);
		ker_F[i].setArg(6, tbuff);
		ker_F[i].setArg(7, rbuff);
		ker_F[i].setArg(8, vbuff);
		ker_F[i].setArg(9, wbuff);
		int a = l3[i];
		int b = l4[i];
		//std::cout << a << "," << b << "\n";
		ker_F[i].setArg(10, vincbuff[a*(n/n_block[0])+b]);
		ker_F[i].setArg(11, wincbuff[a*(n/n_block[0])+b]);
		ker_F[i].setArg(14, Vincbuff[a*(n/n_block[0])+b]);
		ker_F[i].setArg(15, Intincbuff[a*(n/n_block[0])+b]);
		ker_F[i].setArg(12, vincbuff[b*(n/n_block[0])+a]);
		ker_F[i].setArg(13, wincbuff[b*(n/n_block[0])+a]);
		ker_F[i].setArg(16, Vincbuff[b*(n/n_block[0])+a]);
		ker_F[i].setArg(17, Intincbuff[b*(n/n_block[0])+a]);			
		ker_F[i].setArg(20, offset_buff_0[i]);
		ker_F[i].setArg(18, Ftmp);
		ker_F[i].setArg(19, nbuff);
	}
	for (int a = 0; a < n/n_block[0]; a++)
	{
		for (int b = 0; b < n/n_block[0]; b++)
		{
		
			ker_S_interm[a*(n/n_block[0])+b].setArg(0, vincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[a*(n/n_block[0])+b].setArg(1, wincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[a*(n/n_block[0])+b].setArg(6, Vincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[a*(n/n_block[0])+b].setArg(8, Intincbuff[a*(n/n_block[0])+b]);
			ker_S_interm[a*(n/n_block[0])+b].setArg(9, offset_buff_1[a*(n/n_block[0])+b]);	
			ker_S_interm[a*(n/n_block[0])+b].setArg(2, accelsumbuff);
			ker_S_interm[a*(n/n_block[0])+b].setArg(3, alphasumbuff);
			ker_S_interm[a*(n/n_block[0])+b].setArg(4, nbuff);
			ker_S_interm[a*(n/n_block[0])+b].setArg(5, Vsumbuff);
			ker_S_interm[a*(n/n_block[0])+b].setArg(7, Intsumbuff);

			ker_Sp[a*(n/n_block[0])+b].setArg(0, vincbuff[a*(n/n_block[0])+b]);
			ker_Sp[a*(n/n_block[0])+b].setArg(1, wincbuff[a*(n/n_block[0])+b]);
			ker_Sp[a*(n/n_block[0])+b].setArg(3, Vincbuff[a*(n/n_block[0])+b]);
			ker_Sp[a*(n/n_block[0])+b].setArg(4, Intincbuff[a*(n/n_block[0])+b]);	
			ker_Sp[a*(n/n_block[0])+b].setArg(2, nbuff);

		}
	}

	
	ker_Sp0.setArg(0, accelsumbuff);
	ker_Sp0.setArg(1, alphasumbuff);
	ker_Sp0.setArg(2, nbuff);
	ker_Sp0.setArg(3, Vsumbuff);
	ker_Sp0.setArg(4, Intsumbuff);		
	
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

	ker_0_0.setArg(0, accelbuff);
	ker_0_0.setArg(1, Vbuff);
	ker_0_1.setArg(0, alphabuff);
	ker_0_1.setArg(1, Intbuff);

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
	std::vector<cl::NDRange> gsize2 = { n_el, ::size_t(n_block[2]*n_block[1]), ::size_t(n_block[0]*n_block[0]) };
	cl::NDRange local_size(workgroup_size);
	std::cout << "Kernels & Buffers set...";

		
	
	// Initialise timestep scaler	
	float v_norm[n] = { 0 };

	float v_max = 0;


	for (int z = 0; z < n; z++)
	{		
		v_norm[z] = sqrt(pow(v[z][0], 2) + pow(v[z][1], 2) + pow(v[z][2], 2));
		if (z > 0){ v_max = std::max(v_norm[z], v_norm[z - 1]); }		
	}
	
	float F1[4] = { 0 };
	
		
	//queue.enqueueNDRangeKernel(ker_F, offset, gsize2, local_size);
	
	//queue.enqueueReadBuffer(Ftmp, CL_TRUE, ::size_t (0), ::size_t (32), &F1);
	
	float scaleset[4] = { F1[1], F1[1], v_max, v_max };
	
	
	
	cl::Buffer Fbuff(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ::size_t(16), scaleset);
	ker_F[0].setArg(18, Fbuff);
	ker_v_0.setArg(2, Fbuff);
	ker_v_1.setArg(2, Ftmp);
	ker_scale.setArg(0, tbuff);
	ker_scale.setArg(1, Fbuff);
	
		

	// Initialise simulation
	float t_now = 0;
	float t_last = 0;
	short int prog = 0;
	//long counter = 0;
	std::string tempstring;

	queue[0].enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
	queue[0].enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things
	
	tempstring = arraytostring(r,n);
	r_tracker << tempstring;

	tempstring = arraytostring(v, n);
	v_tracker << tempstring;

	std::cout << "..simulation started.\n";
	//for (int i=0; i<100; i++){std::cout << "-";}
	std::cout << "\n";
	//clock_t t0 = clock();
	//clock_t t_temp = t0;
	
	time_t t0 = time(NULL);

	for(int init_x=0; init_x<512; init_x ++)
	{
		//std::cout << init_x << "\n";		
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		//std::cout << "a";

		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size);	// Rotational Kick
		//std::cout << "b";

		queue[0].enqueueNDRangeKernel(ker_T, offset, gsize1, local_size); 		// Evaluate Kinetic Energy
		queue[0].finish();						
		//std::cout << "c";


		if (( t_now == 0 || (t_now - t_last) >= (1.0 / 64.0)*warp) && framecount < n_frames)
		{
			framecount++;
			if (floor(100 * framecount / n_frames) > 4+floor(prog))
			{
				prog = floor(100 * framecount / n_frames); 
				std::cout << prog << "%\n";
			}		
			
			
			queue[0].enqueueReadBuffer(rbuff, CL_TRUE, ::size_t (0), vecsize, r);
			tempstring = arraytostring(r,n);
			r_tracker << tempstring;

			queue[0].enqueueReadBuffer(Vbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			V_tracker << tempstring;

			queue[0].enqueueReadBuffer(Intbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			E_tracker << tempstring;

			queue[0].enqueueReadBuffer(Tvbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tv_tracker << tempstring;
		
			queue[0].enqueueReadBuffer(Twbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tw_tracker << tempstring;

			queue[0].enqueueReadBuffer(vbuff, CL_TRUE, ::size_t (0), vecsize, v);
			tempstring = arraytostring(v, n);
			v_tracker << tempstring;
		
			queue[0].enqueueReadBuffer(wbuff, CL_TRUE, ::size_t (0), vecsize, w);
			tempstring = arraytostring(w, n);
			w_tracker << tempstring;
			

			t_last = t_now;
			
		}

		//queue.enqueueReadBuffer(tbuff, CL_TRUE, ::size_t(0), sizeof(stuff), stuff);
		//std::cout << stuff[0] << ", ";
		t_now += stuff[0];
		//queue.enqueueNDRangeKernel(ker_scale,offset,local_size,local_size); 	// Set new time step
		
		queue[0].enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
		queue[0].enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things
		
	
		queue[0].enqueueNDRangeKernel(ker_r, offset, gsize1, local_size); 		// Drift
		//std::cout << "a";

		n0[3] = n_block[0];
		n0[2] = n_block[0];
		n0[1] = n_block[0];

		for (int i = 0; i < nDevs; i++)	{queue[i].enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);}		

		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]); i++)
		{
			int ind = i - nDevs*floor(i/nDevs);
			
								
			if (l6[i] == -1) 
			{
				int a = l3[l5[i]];
				int b = l4[l5[i]];
				try {queue[ind].enqueueNDRangeKernel(ker_F[l5[i]], offset, gsize2[2], local_size);}
				catch(cl::Error e) {std::cout << "1:" << e.what() << "," << e.err() << "\n";break;}  		// Compute force
				
				n0[2] = n0[1];				
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue[ind].enqueueNDRangeKernel(ker_Sp[a*(n/n_block[0])+b], offset, gsizeRed, local_size);
					queue[ind].enqueueNDRangeKernel(ker_Sp[b*(n/n_block[0])+a], offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}				
			}	
			else  
			{	
				int a = l3[l5[i]];
				int b = l3[l6[i]];		
				try {queue[ind].enqueueNDRangeKernel(ker_F[l5[i]], offset, gsize2[1], local_size);}
				catch(cl::Error e) {std::cout << "2:" << e.what() << "," << e.err() << "\n";break;} 
				try {queue[ind].enqueueNDRangeKernel(ker_F[l6[i]], offset, gsize2[1], local_size);}
				catch(cl::Error e) {std::cout << "3:" << e.what() << "," << e.err() << "\n";break;}  	 					
				
				n0[2] = n0[1];				
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue[ind].enqueueNDRangeKernel(ker_Sp[a*(n/n_block[0])+a], offset, gsizeRed, local_size);
					queue[ind].enqueueNDRangeKernel(ker_Sp[b*(n/n_block[0])+b], offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}		
				
			}									
		}
		//std::cout << "b";
		
		n0[1] = n;
		n0[2] = n/n_block[0];
		n0[3] = n;
		for (int i = 0; i < nDevs; i++)	{queue[i].enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);}
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]); i++)
		{					
			int ind = i - nDevs*floor(i/nDevs);
			cl::NDRange gsizeRed(n_block[0]);
	
			if (l6[i] == -1)
			{
				int a = l3[l5[i]];
				int b = l4[l5[i]];
				queue[ind].enqueueNDRangeKernel(ker_S_interm[a*(n/n_block[0])+b], offset, gsizeRed, local_size);
				queue[ind].enqueueNDRangeKernel(ker_S_interm[b*(n/n_block[0])+a], offset, gsizeRed, local_size);						
			}
			else
			{	
				int a = l3[l5[i]];
				int b = l3[l6[i]];							
				queue[ind].enqueueNDRangeKernel(ker_S_interm[a*(n/n_block[0])+a], offset, gsizeRed, local_size);
				queue[ind].enqueueNDRangeKernel(ker_S_interm[b*(n/n_block[0])+b], offset, gsizeRed, local_size);
			}	
		}


		//std::cout << "d";


		n0[1] = n/n_block[0];
		
		for (int i = 0; i < nDevs; i++)	{queue[i].enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);}		
	
		while(n0[2] > 1)
		{			
			cl::NDRange gsizeRed(0.5*n*n0[2]);		
			queue[0].enqueueNDRangeKernel(ker_Sp0, offset, gsizeRed, local_size);			
			n0[2] = 0.5*n0[2];			
		}		
		
		//std::cout << "e\n";

		queue[0].enqueueNDRangeKernel(ker_S, offset, gsize1, local_size);

		
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size); 	// Rotational Kick

		queue[0].enqueueNDRangeKernel(ker_t, offset, gsize1m, unitsize);		// Make positions relative to particle 1
		queue[0].enqueueNDRangeKernel(ker_t0, offset, unitsize, unitsize);	
		
		queue[0].finish();		
	}
	
	time_t t_elap = difftime(time(NULL),t0);	
	float est_time = (1.0/512.0)*float(t_elap)*(float(max_time)/stuff[0]);
	int est_h = floor(est_time/3600);
	int est_m = floor((est_time/60)-60*est_h);
	int est_s = floor(est_time-60*est_m-3600*est_h);
	std::cout << "First 512 steps runtime: " << float(t_elap) << "s; Estimated run time: " << est_h << "h" << est_m << "m" << est_s << "s\n";

	while (t_now < max_time)
	{
		//std::cout << init_x << "\n";		
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		//std::cout << "a";

		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size);	// Rotational Kick
		//std::cout << "b";

		queue[0].enqueueNDRangeKernel(ker_T, offset, gsize1, local_size); 		// Evaluate Kinetic Energy
		queue[0].finish();						
		//std::cout << "c";


		if (( t_now == 0 || (t_now - t_last) >= (1.0 / 64.0)*warp) && framecount < n_frames)
		{
			framecount++;
			if (floor(100 * framecount / n_frames) > 4+floor(prog))
			{
				prog = floor(100 * framecount / n_frames); 
				std::cout << prog << "%\n";
			}		
			
			
			queue[0].enqueueReadBuffer(rbuff, CL_TRUE, ::size_t (0), vecsize, r);
			tempstring = arraytostring(r,n);
			r_tracker << tempstring;

			queue[0].enqueueReadBuffer(Vbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			V_tracker << tempstring;

			queue[0].enqueueReadBuffer(Intbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			E_tracker << tempstring;

			queue[0].enqueueReadBuffer(Tvbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tv_tracker << tempstring;
		
			queue[0].enqueueReadBuffer(Twbuff, CL_TRUE, ::size_t (0), ::size_t(4*n), E_temp);
			tempstring = arraytostring(E_temp, n);
			Tw_tracker << tempstring;

			queue[0].enqueueReadBuffer(vbuff, CL_TRUE, ::size_t (0), vecsize, v);
			tempstring = arraytostring(v, n);
			v_tracker << tempstring;
		
			queue[0].enqueueReadBuffer(wbuff, CL_TRUE, ::size_t (0), vecsize, w);
			tempstring = arraytostring(w, n);
			w_tracker << tempstring;
			

			t_last = t_now;
			
		}

		//queue.enqueueReadBuffer(tbuff, CL_TRUE, ::size_t(0), sizeof(stuff), stuff);
		//std::cout << stuff[0] << ", ";
		t_now += stuff[0];
		//queue.enqueueNDRangeKernel(ker_scale,offset,local_size,local_size); 	// Set new time step
		
		queue[0].enqueueNDRangeKernel(ker_0_0, offset, gsize1, local_size); 	// zero things
		queue[0].enqueueNDRangeKernel(ker_0_1, offset, gsize1, local_size); 	// zero things
		
	
		queue[0].enqueueNDRangeKernel(ker_r, offset, gsize1, local_size); 		// Drift
		//std::cout << "a";

		n0[3] = n_block[0];
		n0[2] = n_block[0];
		n0[1] = n_block[0];

		for (int i = 0; i < nDevs; i++)	{queue[i].enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);}		

		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]); i++)
		{
			int ind = i - nDevs*floor(i/nDevs);
			
								
			if (l6[i] == -1) 
			{
				int a = l3[l5[i]];
				int b = l4[l5[i]];
				try {queue[ind].enqueueNDRangeKernel(ker_F[l5[i]], offset, gsize2[2], local_size);}
				catch(cl::Error e) {std::cout << "1:" << e.what() << "," << e.err() << "\n";break;}  		// Compute force
				
				n0[2] = n0[1];				
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue[ind].enqueueNDRangeKernel(ker_Sp[a*(n/n_block[0])+b], offset, gsizeRed, local_size);
					queue[ind].enqueueNDRangeKernel(ker_Sp[b*(n/n_block[0])+a], offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}				
			}	
			else  
			{	
				int a = l3[l5[i]];
				int b = l3[l6[i]];		
				try {queue[ind].enqueueNDRangeKernel(ker_F[l5[i]], offset, gsize2[1], local_size);}
				catch(cl::Error e) {std::cout << "2:" << e.what() << "," << e.err() << "\n";break;} 
				try {queue[ind].enqueueNDRangeKernel(ker_F[l6[i]], offset, gsize2[1], local_size);}
				catch(cl::Error e) {std::cout << "3:" << e.what() << "," << e.err() << "\n";break;}  	 					
				
				n0[2] = n0[1];				
				while (n0[2] > 1)
				{			
					cl::NDRange gsizeRed(0.5*n0[1]*n0[2]);
					queue[ind].enqueueNDRangeKernel(ker_Sp[a*(n/n_block[0])+a], offset, gsizeRed, local_size);
					queue[ind].enqueueNDRangeKernel(ker_Sp[b*(n/n_block[0])+b], offset, gsizeRed, local_size);
					n0[2] = 0.5*n0[2];
				}		
				
			}									
		}
		//std::cout << "b";
		
		n0[1] = n;
		n0[2] = n/n_block[0];
		n0[3] = n;
		for (int i = 0; i < nDevs; i++)	{queue[i].enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);}
		
		for (int i = 0; i < 0.5*(n/n_block[0])*(n/n_block[0]); i++)
		{					
			int ind = i - nDevs*floor(i/nDevs);
			cl::NDRange gsizeRed(n_block[0]);
	
			if (l6[i] == -1)
			{
				int a = l3[l5[i]];
				int b = l4[l5[i]];
				queue[ind].enqueueNDRangeKernel(ker_S_interm[a*(n/n_block[0])+b], offset, gsizeRed, local_size);
				queue[ind].enqueueNDRangeKernel(ker_S_interm[b*(n/n_block[0])+a], offset, gsizeRed, local_size);						
			}
			else
			{	
				int a = l3[l5[i]];
				int b = l3[l6[i]];							
				queue[ind].enqueueNDRangeKernel(ker_S_interm[a*(n/n_block[0])+a], offset, gsizeRed, local_size);
				queue[ind].enqueueNDRangeKernel(ker_S_interm[b*(n/n_block[0])+b], offset, gsizeRed, local_size);
			}	
		}


		//std::cout << "d";


		n0[1] = n/n_block[0];
		
		for (int i = 0; i < nDevs; i++)	{queue[i].enqueueWriteBuffer(nbuff, CL_TRUE, ::size_t (0), sizeof(n0), n0);}		
	
		while(n0[2] > 1)
		{			
			cl::NDRange gsizeRed(0.5*n*n0[2]);		
			queue[0].enqueueNDRangeKernel(ker_Sp0, offset, gsizeRed, local_size);			
			n0[2] = 0.5*n0[2];			
		}		
		
		//std::cout << "e\n";

		queue[0].enqueueNDRangeKernel(ker_S, offset, gsize1, local_size);

		
		queue[0].enqueueNDRangeKernel(ker_v_0, offset, gsize1, local_size); 	// Translational Kick
		queue[0].enqueueNDRangeKernel(ker_v_1, offset, gsize1, local_size); 	// Rotational Kick

		queue[0].enqueueNDRangeKernel(ker_t, offset, gsize1m, unitsize);		// Make positions relative to particle 1
		queue[0].enqueueNDRangeKernel(ker_t0, offset, unitsize, unitsize);	
		
		queue[0].finish();		
	}
  	

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
	
	//long hold = 0;
	//while (true)
	//{
		//hold++;
		//hold--;
	//}
	return 0;
}
